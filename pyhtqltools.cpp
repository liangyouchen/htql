
#include "pyhtqltools.h"
#include "htmlql.h"
#include "RegExParser.h"
#include "alignment.h"
#include "HtTextAlign.h"
#include "pyhtqlmodule.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_THIS_FILE
#endif


//In LINKING: use Force File output to produce the pyd file

/*
DEBUGG: copy the file python26.dll to python26_d.dll. 
		edit pyconfig.h and comment out the line "#define Py_DEBUG" (line 374)

  */



void PyTools::dealloc(PyTools* self){
	if (self->Align) delete self->Align;
	self->Align=0; 

#ifdef Python27
    self->ob_type->tp_free((PyObject*)self);
#else
	Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

PyObject* PyTools::alloc(PyTypeObject *type, PyObject *args, PyObject *kwds){
    PyTools *self;

    self = (PyTools *)type->tp_alloc(type, 0);

    if (self != NULL) {
		self->Align=new HtTextAlign; 
	}

    return (PyObject *)self;
}

int PyTools::init(PyTools *self, PyObject *args, PyObject *kwds){

    return 0;
}


PyObject* PyTools::cluster(PyTools* self, PyObject *args, PyObject *kwds){
	PyObject* text=0;
    static char *kwlist[] = {"text", "is_html", NULL};
	int is_html=0;

#ifdef _DEBUG
	debug_str("before PyArg_ParseTupleAndKeywords"); 
#endif
	
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &text, &is_html))
        return 0; 

#ifdef _DEBUG
	debug_str("before PyList_Size"); 
#endif

	long len=PyList_Size(text); 
	long i; 
	PyObject* item; 
	ReferData str;
	for (i=0; i<len; i++){
#ifdef _DEBUG
		debug_str("before PyList_GetItem"); 
#endif

		item=PyList_GetItem(text, i);
		uni2str(item, &str);
		if (is_html){
			self->Align->setPlainText(i+1, &str);
		}else{
			self->Align->setHtmlText(i+1, &str);
		}
	}

	self->Align->insertClusterRow(0, 1,2);
	for (i=2; i<len; i++){
		self->Align->addToClusters(i+1); 
	}

#ifdef _DEBUG
	debug_str("before malloc"); 
#endif
	long nfields=self->Align->ClusterN;

	PyObject* result=PyList_New(nfields);
	for (i=0; i<nfields; i++){
		PyObject* tuple=PyTuple_New(2);
		PyTuple_SetItem(tuple, 0, PyLong_FromLong(self->Align->Clusters(i,0)-1) );
		PyTuple_SetItem(tuple, 1, PyLong_FromLong(self->Align->Clusters(i,1)-1) );

		PyList_SetItem(result, i, tuple);
	}

	return result;
}
PyObject* PyTools::getAlignment(PyTools* self, PyObject *args, PyObject *kwds){
	if (!self) return 0; 

	ReferLinkHeap aligned_strs; 
	long aligned_len=0; 
	double cost=0; 

	self->Align->alignMultiple(&aligned_strs, &aligned_len, &cost);
	

	long nfields=aligned_strs.Total;


	PyObject* result=PyList_New(nfields);
	long i=0;
	char* empty_char="-";
	for (ReferLink* link=(ReferLink*) aligned_strs.moveFirst(); link; link=(ReferLink*) aligned_strs.moveNext() ){
		ReferLink** p=(ReferLink**) link->Value.P; 

		PyObject* tuple=PyList_New(aligned_len);
		for (long j=0; j<aligned_len; j++){
			PyList_SetItem(tuple, j, p[j] ? str2uni(((ReferLink*)(p[j]->Name.P ))->Name.P) : str2uni(empty_char) );
		}

		PyList_SetItem(result, i++, tuple);
	}

	return result;
}
PyObject* PyTools::regSearchStr(PyTools* self, PyObject *args, PyObject *kwds){
	char* text, *regex; 
 	int case_sensitive=true;
	int overlapping=false; 
	int useindex=false;
	static char *kwlist[] = {"text", "regex", "case", "overlap", "useindex", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "ss|iii", kwlist, &text, &regex, &case_sensitive, &overlapping, &useindex))
        return 0; 

	ReferLinkHeap results;
	results.setSortOrder(SORT_ORDER_NUM_INC);
	ReferLink* link;
	RegExParser regparser;
	regparser.CaseSensitive=case_sensitive;
	regparser.Overlap=overlapping;
	regparser.searchRegExText(text, regex, &results);

	PyObject* result=PyList_New(results.Total);
	long i=0, count=0;
	for (link=(ReferLink*) results.moveFirst(); link; link=(ReferLink*) results.moveNext()){
		PyObject* tuple=0;
		if (useindex){
			tuple=PyTuple_New(2);
			PyTuple_SetItem(tuple, 0, PyLong_FromLong(link->Value.P-text) );
			PyTuple_SetItem(tuple, 1, PyLong_FromLong(link->Value.L) );
		}else{
			link->Value.Seperate();
			tuple=str2uni(link->Value.P);
		}
		
		PyList_SetItem(result, count++, tuple);
	}

	return result;
}
PyObject* PyTools::regSearchList(PyTools* self, PyObject *args, PyObject *kwds){
	PyObject* text=0;
	char* regex=0;
	int case_sensitive=true;
	int overlapping=false; 
	int useindex=false;
    static char *kwlist[] = {"text", "regex","case", "overlap", "useindex", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "Os|iii", kwlist, &text, &regex, &case_sensitive, &overlapping, &useindex))
        return 0; 

	ReferLinkHeap results;
	results.setSortOrder(SORT_ORDER_NUM_INC);
	ReferLink* link;
	
	//convert python list to tokens
	PyObject* item, *item1; 
	ReferData str;
	RegExToken text_start, regex_start; 
	RegExToken* token=&text_start;
	RegExToken* token1;
	ReferLinkHeap text_tokens; 
	text_tokens.setSortOrder(SORT_ORDER_NUM_INC);
	long i, j; 
	long count=0;
	long index=0;
	long len=PyList_Size(text); 
	//debug_str("==text==", true); 
	for (i=0; i<len; i++){
		item=PyList_GetItem(text, i);
#ifdef Python27
		if (PyString_Check(item)){
#else
		if (PyUnicode_Check(item)){
#endif
			uni2str(item, &str);
			link=text_tokens.add(&str, 0, count++); 

			token=token->addNextToken(new RegExToken);
			token->Token.Set(link->Name.P, link->Name.L, false); 
			token->TokenText.Set(link->Name.P, link->Name.L, true); 
			token->Index=index++;
		}else if (PyList_Check(item) || PyTuple_Check(item)) { //a tuple
			long len1=0; 
			int is_list=0;
			if (PyList_Check(item)){
				len1=PyList_Size(item);
				is_list=1;
			}else if (PyTuple_Check(item))
				len1=PyTuple_Size(item); 

			for (j=0; j<len1; j++){
				if (is_list) item1=PyList_GetItem(item, j);
				else item1=PyTuple_GetItem(item, j);

				uni2str(item1, &str);
				link=text_tokens.add(&str, 0, count++); 
				
				if (j==0){
					token=token->addNextToken(new RegExToken);
					token1=token;
					token1->Index=index++; 
				}else{
					token1=token1->addExtendedToken(new RegExToken);
					//token1->Index=j*len+index-1;
					token1->Index=index-1;
				}
				token1->Token.Set(link->Name.P, link->Name.L, false); 
				token1->TokenText.Set(link->Name.P, link->Name.L, true); 
			}
		}
		//debug_str(token->Token.P);
	}
	token=token->addNextToken(new RegExToken);
	token->Index=index;

	//parse regular expression to structure regex_start
	ListRegExParser regparser;
	regparser.CaseSensitive=case_sensitive;
	regparser.Overlap=overlapping;
	RegExSyntax syntax; 
	syntax.setSentence(regex);
	int err=regparser.parseRegEx(&syntax, &regex_start); 

	//search text with regular expression
	err=regparser.searchRegExTokens(text_start.Next, &regex_start, &results);


	RegExToken* matched_to=0; 
	PyObject* result=PyList_New(results.Total);
	count=0;
	for (link=(ReferLink*) results.moveFirst(); link; link=(ReferLink*) results.moveNext()){
		PyObject* tuple=0;
		if (useindex){ 
			// (ii)
			tuple=PyTuple_New(2);

			matched_to=(RegExToken*) link->Name.P; 
			PyTuple_SetItem(tuple, 0, PyLong_FromLong(matched_to->Index) );

			j=0;
			for (matched_to=(RegExToken*) link->Name.P; matched_to && matched_to!=(RegExToken*) link->Value.P; matched_to=matched_to->Next){
				j++;
			}
			PyTuple_SetItem(tuple, 1, PyLong_FromLong(j) );
		}else{
			j=0;
			for (matched_to=(RegExToken*) link->Name.P; matched_to && matched_to!=(RegExToken*) link->Value.P; matched_to=matched_to->Next){
				j++;
			}
			tuple=PyList_New(j);
			j=0;
			for (matched_to=(RegExToken*) link->Name.P; matched_to && matched_to!=(RegExToken*) link->Value.P; matched_to=matched_to->Next){
				PyObject* tuple1=0;
				if (matched_to->ExtendNext){
					i=1;
					for (token=matched_to->ExtendNext; token; token=token->ExtendNext){
						i++;
					}

					tuple1=PyTuple_New(i);
					PyTuple_SetItem(tuple1, 0, str2uni(matched_to->TokenText.P));
					i=1;
					for (token=matched_to->ExtendNext; token; token=token->ExtendNext){
						PyTuple_SetItem(tuple1, i++, str2uni(token->TokenText.P));
					}				
				}else{
					tuple1=str2uni(matched_to->TokenText.P);
				}

				PyList_SetItem(tuple, j++, tuple1);
			}

		}
		PyList_SetItem(result, count++, tuple);
	}

	return result;
}




