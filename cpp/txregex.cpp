
#include "txregex.h"
#include "referlink.h"
#include "stroper.h"
#include "pyhtqlmodule.h"
#include "RegExParser.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_THIS_FILE
#endif



//In LINKING: use Force File output to produce the pyd file

/*
DEBUG: copy the file python26.dll to python26_d.dll. 
		edit pyconfig.h and comment out the line "#define Py_DEBUG" (line 374)

  */



void TxRegEx::dealloc(TxRegEx* self){
	if (self->RegContext) delete self->RegContext;
	self->RegContext = 0;
	if (self->TextStart) delete self->TextStart;
	self->TextStart=0;
	if (self->KeptText) delete self->KeptText; 
	self->KeptText=0;
	if (self->KeptRegEx) delete self->KeptRegEx;
	self->KeptRegEx = 0;
	self->clearRegExpr();
	if (self->RegExprSentence) delete self->RegExprSentence; 
	self->RegExprSentence = 0;

#ifdef Python27
    self->ob_type->tp_free((PyObject*)self);
#else
	Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

PyObject* TxRegEx::alloc(PyTypeObject *type, PyObject *args, PyObject *kwds){
    TxRegEx *self;

    self = (TxRegEx *)type->tp_alloc(type, 0);
    if (self) {
		self->RegContext = new RegExParser;
		self->KeptText = new ReferLinkHeap;
		self->KeptText->setSortOrder(SORT_ORDER_NUM_INC);
		self->KeptText->setDuplication(true);
		self->TextStart = 0;
		self->RegExpr = 0;
		self->RegParser = 0;
		self->IsRegList = 0;
		self->IsKeptRegEx=0;
		self->KeptRegEx = new RegExKeeper;
		self->RegExprSentence = new ReferData; 
    }

    return (PyObject *)self;
}

int TxRegEx::init(TxRegEx *self, PyObject *args, PyObject *kwds){
	if (self && !self->RegContext) {
		self->RegContext=new RegExParser;
		self->KeptText = new ReferLinkHeap;
		self->KeptText->setSortOrder(SORT_ORDER_NUM_INC);
		self->KeptText->setDuplication(true);
		self->TextStart=0;
		self->RegExpr = 0; 
		self->RegParser = 0;
		self->IsRegList=0;
		self->IsKeptRegEx=0;
		self->KeptRegEx = new RegExKeeper;
	}
    return 0;
}
int TxRegEx::clearRegExpr(){
	if (!IsKeptRegEx){
		if (RegExpr) delete RegExpr;
		if (RegParser) delete RegParser; 
	}
	RegExpr = 0;
	RegParser = 0;
	IsRegList = 0;
	IsKeptRegEx = 0;
	return 0;
}
int TxRegEx::useKeptRegEx(ReferData* tuple){
	clearRegExpr();
	RegExpr = (RegExToken*) tuple[RegExKeeper::ID_RegEx].P; 
	RegParser = (RegExParser*) tuple[RegExKeeper::ID_Parser].P; 
	IsRegList = tuple[RegExKeeper::ID_IsList].L; 
	IsKeptRegEx = true; 
	return 0;
}

int TxRegEx::parseRegExpr(const char* sentence, int is_list, int case_sensitive, int keep, const char* keepname){
	int err=0;
	ReferData keepname1;
	RegExprSentence->Set((char*) sentence, sentence?strlen(sentence):0, true);
	keepname1.Set((char*) keepname, keepname?strlen(keepname):0, false);

#ifdef DEBUG_THIS_FILE
	ReferData debug_info; 
	debug_info="parseRegExpr: ";
	debug_info+=sentence;
	debug_info+="\r\n";
	debug_info.appendFile("$DEBUG_RegExParser.txt");
#endif

	ReferData* tuple=0;
	int to_keep=keep || (keepname && keepname[0]);
	if (to_keep){
		tuple=KeptRegEx->findRegEx(&keepname1, RegExprSentence);
		if (tuple) useKeptRegEx(tuple); 
	}
	if (!tuple){
		clearRegExpr();
		RegExpr = new RegExToken;
		if (is_list) RegParser=new ListRegExParser; 
		else RegParser=new RegExParser;

		IsRegList = is_list; 

		ReferLink* link=KeptText->add(sentence, 0, KeptText->Total);
		RegExSyntax syntax; 
		syntax.setSentence(link->Name.P, &link->Name.L, false); //should use the keep text

		RegParser->Context = RegContext; 
		RegParser->CaseSensitive = case_sensitive; 
		err=RegParser->parseRegEx(&syntax, RegExpr);

		if (to_keep){
			tuple = KeptRegEx->keepRegEx(&keepname1, RegExprSentence, RegExpr, RegParser, IsRegList);
			IsKeptRegEx = true; 
		}
	}

	return err;
}

int TxRegEx::tokenizeListText(PyObject* text, RegExToken* token_text, long limit){
	int err=0;
	ReferData str;
	ReferLink* link;
	PyObject* item, *item1; 
	RegExToken* token=token_text;
	RegExToken* token1;
	long i, j; 
	long count=0;
	long index=0;
	long len=PyList_Size(text); 
	if (limit>=0 && len>limit) len=limit; 
	//debug_str("==text==", true); 
	for (i=0; i<len; i++){
		item=PyList_GetItem(text, i);
#ifdef Python27
		if (PyString_Check(item)){
#else
		if (PyUnicode_Check(item)){
#endif
			uni2str(item, &str);
			link=KeptText->add(&str, 0, KeptText->Total); 

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
				link=KeptText->add(&str, 0, KeptText->Total); 
				
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
				token1->TokenType=RegExSyntax::synRegQL_TAG;
			}
		}
		//debug_str(token->Token.P);
	}
	token=token->addNextToken(new RegExToken);
	token->Index=index;

	return err;
}
PyObject* TxRegEx::buildListResultsSet(ReferSet* results, int useindex, int withgroup){
	RegExToken* matched_to=0; 
	PyObject* result=PyList_New(results->TupleCount);
	long count=0;
	long i, j, ifield;
	RegExToken* token;
	ReferData* result1;
	for (result1=(ReferData*) results->moveFirst(); result1; result1=(ReferData*) results->moveNext()){
		PyObject* tuple=PyList_New(results->FieldsNum); 
		for (ifield=0; ifield<results->FieldsNum; ifield++){
			PyObject* rtuple=0;
			if (useindex){ 
				// (ii)
				rtuple=PyTuple_New(2);

				matched_to=(RegExToken*) result1[ifield].P; 
				PyTuple_SetItem(rtuple, 0, PyLong_FromLong(matched_to->Index) );

				j=0;
				for (matched_to=(RegExToken*) result1[ifield].P; matched_to && matched_to!=(RegExToken*) result1[ifield].L; matched_to=matched_to->Next){
					j++;
				}
				PyTuple_SetItem(rtuple, 1, PyLong_FromLong(j) );
			}else{
				j=0;
				for (matched_to=(RegExToken*) result1[ifield].P; matched_to && matched_to!=(RegExToken*) result1[ifield].L; matched_to=matched_to->Next){
					j++;
				}
				rtuple=PyList_New(j);
				j=0;
				for (matched_to=(RegExToken*) result1[ifield].P; matched_to && matched_to!=(RegExToken*) result1[ifield].L; matched_to=matched_to->Next){
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

					PyList_SetItem(rtuple, j++, tuple1);
				}
			}		
			PyList_SetItem(tuple, ifield, rtuple);
		}
		if (withgroup){
			PyList_SetItem(result, count++, tuple);
		}else{
			PyObject* rtuple=PyList_GetItem(tuple, 0);
			Py_INCREF(rtuple);
			PyList_SetItem(result, count++, rtuple );
			Py_DECREF(tuple); 
			tuple=0;
		}
	}

	return result;
}
PyObject* TxRegEx::buildListResults(ReferLinkHeap* results, int useindex){
	RegExToken* matched_to=0; 
	PyObject* result=PyList_New(results->Total);
	long count=0;
	long i, j;
	ReferLink* link;
	RegExToken* token;
	for (link=(ReferLink*) results->moveFirst(); link; link=(ReferLink*) results->moveNext()){
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
PyObject* TxRegEx::buildStrResultsSet(ReferSet* results, int useindex, int withgroup){
	PyObject* result=PyList_New(results->TupleCount);
	long i=0, count=0;
	ReferData* result1;
	for (result1=(ReferData*) results->moveFirst(); result1; result1=(ReferData*) results->moveNext()){
		PyObject* tuple=PyList_New(results->FieldsNum); 
		for (i=0; i<results->FieldsNum; i++){
			RegExToken* token_from=(RegExToken*) result1[i].P; 
			RegExToken* token_to=(RegExToken*) result1[i].L; 
			PyObject* rtuple=0;
			if (useindex){
				rtuple=PyTuple_New(2);
				PyTuple_SetItem(rtuple, 0, PyLong_FromLong(token_from->Index) );
				PyTuple_SetItem(rtuple, 1, PyLong_FromLong(token_to->Index - token_from->Index) );
			}else{
				ReferData str;
				str.Set(token_from->TokenText.P, (token_to->TokenText.P-token_from->TokenText.P), true);
				rtuple=str2uni(str.P);
			}
			PyList_SetItem(tuple, i, rtuple);
		}
		if (withgroup){
			PyList_SetItem(result, count++, tuple);
		}else{
			PyObject* rtuple=PyList_GetItem(tuple, 0);
			Py_INCREF(rtuple);
			PyList_SetItem(result, count++, rtuple );
			Py_DECREF(tuple); 
			tuple=0;
		}
	}

	return result;
}
PyObject* TxRegEx::buildStrResults(ReferLinkHeap* results, int useindex){
	PyObject* result=PyList_New(results->Total);
	long i=0, count=0;
	ReferLink* link;
	for (link=(ReferLink*) results->moveFirst(); link; link=(ReferLink*) results->moveNext()){
		RegExToken* token_from=(RegExToken*) link->Name.P; 
		RegExToken* token_to=(RegExToken*) link->Value.P; 
		PyObject* tuple=0;
		if (useindex){
			tuple=PyTuple_New(2);
			PyTuple_SetItem(tuple, 0, PyLong_FromLong(token_from->Index) );
			PyTuple_SetItem(tuple, 1, PyLong_FromLong(token_to->Index - token_from->Index) );
		}else{
			ReferData str;
			str.Set(token_from->TokenText.P, (token_to->TokenText.P-token_from->TokenText.P), true);
			tuple=str2uni(str.P);
		}
		
		PyList_SetItem(result, count++, tuple);
	}

	return result;
}
PyObject* TxRegEx::searchRegTextStr(const char*text, int overlapping, int useindex, int group, long limit){
	if (!RegParser) return PyErr_Format(PyExc_RuntimeError, "regular expression not set!");
	//if (!RegParser) return PyErr_NewException("htql.RegEx", NULL, NULL);
	if (TextStart) delete TextStart; 
	TextStart = new RegExToken;	
	int err=RegParser->tokenizeText(text, TextStart, limit);

#ifdef DEBUG_THIS_FILE
	ReferData debug_info; 
	debug_info="searchRegTextStr: ";
	debug_info+=text;
	debug_info+="\r\n";
	debug_info.appendFile("$DEBUG_RegExParser.txt");
#endif

	RegParser->Overlap = overlapping;
	if (group){
		ReferSet results;
		err=RegParser->searchRegExTokensSet(TextStart->Next, RegExpr, &results, false, group);//group
		return buildStrResultsSet(&results, useindex, group);
	}else{
		ReferLinkHeap results;
		results.setSortOrder(SORT_ORDER_NUM_INC);
		err=RegParser->searchRegExTokens(TextStart->Next, RegExpr, &results);
		return buildStrResults(&results, useindex);
	}
}
PyObject* TxRegEx::searchRegTextList(PyObject*text, int overlapping, int useindex, int group, long limit){
	if (!RegParser) return PyErr_Format(PyExc_RuntimeError, "regular expression not set!");
	//if (!RegParser) return PyErr_NewException("htql.RegEx", NULL, NULL);
	if (TextStart) delete TextStart; 
	TextStart = new RegExToken;	
	int err=tokenizeListText(text, TextStart, limit);

#ifdef DEBUG_THIS_FILE
	ReferData debug_info; 
	debug_info="searchRegTextStr: ";
	TextStart->getTokenListInfo(&debug_info, 0);
	debug_info+="\r\n";
	debug_info.appendFile("$DEBUG_RegExParser.txt");
#endif

	RegParser->Overlap = overlapping;
	if (group){
		ReferSet results;
		err=RegParser->searchRegExTokensSet(TextStart->Next, RegExpr, &results, false, group);
		return buildListResultsSet(&results, useindex, group);
	}else{
		ReferLinkHeap results;
		results.setSortOrder(SORT_ORDER_NUM_INC);
		err=RegParser->searchRegExTokens(TextStart->Next, RegExpr, &results);
		return buildListResults(&results, useindex);
	}
}
PyObject* TxRegEx::regSearchStr(TxRegEx* self, PyObject *args, PyObject *kwds){
	char* text, *regex; 
 	int case_sensitive=true;
	int overlapping=false; 
	int useindex=false;
	int keep=0;
	int group=0;
	char* keepname=0;
	long limit=-1; 
	static char *kwlist[] = {"text", "regex", "case", "overlap", "useindex", "keep", "group", "keepname", "limit", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "ss|iiiiisi", kwlist, &text, &regex, &case_sensitive, &overlapping, &useindex, &keep, &group, &keepname, &limit))
        return 0; 

#ifdef DEBUG_THIS_FILE
	ReferData debug_info; 
	debug_info="regSearchStr: \r\n\ttext=";
	debug_info+=text;
	debug_info+="\r\n\tregex=";
	debug_info+=regex;
	debug_info+="\r\n";
	debug_info.appendFile("$DEBUG_RegExParser.txt");
#endif

	int err=self->parseRegExpr(regex, 0, case_sensitive, keep, keepname);
	return self->searchRegTextStr(text, overlapping, useindex, group, limit);
}
PyObject* TxRegEx::regSearchList(TxRegEx* self, PyObject *args, PyObject *kwds){
	PyObject* text=0;
	char* regex=0;
	int case_sensitive=true;
	int overlapping=false; 
	int useindex=false;
 	int keep=0;
	int group=0; 
	char* keepname=0;
	long limit=-1; 
   static char *kwlist[] = {"text", "regex","case", "overlap", "useindex", "keep", "group", "keepname", "limit", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "Os|iiiiisi", kwlist, &text, &regex, &case_sensitive, &overlapping, &useindex, &keep, &group, &keepname, &limit))
        return 0; 

#ifdef DEBUG_THIS_FILE
	ReferData debug_info; 
	debug_info="regSearchList: \r\n\tregex=";
	debug_info+=regex;
	debug_info+="\r\n";
	debug_info.appendFile("$DEBUG_RegExParser.txt");
#endif

	int err=self->parseRegExpr(regex, 1, case_sensitive, keep, keepname);
	return self->searchRegTextList(text, overlapping, useindex, group, limit);
}

PyObject* TxRegEx::setExprStr(TxRegEx* self, PyObject *args, PyObject *kwds){
	char *regex=0; 
	int case_sensitive=true;
	int keep=0;
	char* keepname=0;
	static char *kwlist[] = {"regex", "case", "keep", "keepname", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s|iis", kwlist, &regex, &case_sensitive, &keep, &keepname ))
        return 0; 

	int err=self->parseRegExpr(regex, 0, case_sensitive, keep, keepname);
	return PyLong_FromLong(err);
}
PyObject* TxRegEx::setExprList(TxRegEx* self, PyObject *args, PyObject *kwds){
	char *regex; 
	int case_sensitive=true;
	int keep=0;
	char* keepname=0; 
	static char *kwlist[] = {"regex", "case", "keep", "keepname", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s|iis", kwlist, &regex, &case_sensitive, &keep, &keepname))
        return 0; 

	int err=self->parseRegExpr(regex, 1, case_sensitive, keep, keepname);
	return PyLong_FromLong(err);
}
PyObject* TxRegEx::regExprSearch(TxRegEx* self, PyObject *args, PyObject *kwds){
	PyObject* text=0;
	int overlapping=false; 
	int useindex=false;
	char* exprname=0;
	int group=false;
	long limit=-1; 
	static char *kwlist[] = {"text", "overlap", "useindex", "group", "exprname", "limit", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "O|iiisi", kwlist, &text, &overlapping, &useindex, &group, &exprname, &limit))
        return 0; 

	if (exprname){ 
		ReferData fieldvalue;
		fieldvalue.Set(exprname, strlen(exprname), false);
		ReferData* tuple=self->KeptRegEx->KeepSet.findFieldString(self->KeptRegEx->KeepSet.FieldNames[RegExKeeper::ID_Name]->P, &fieldvalue);
		if (tuple) self->useKeptRegEx(tuple); 
	}

	if (self->IsRegList) 
		return self->searchRegTextList(text, overlapping, useindex, group, limit);
	else {
		ReferData text_str; 
		uni2str(text, &text_str);
		return self->searchRegTextStr(text_str.P, overlapping, useindex, group, limit);
	}
}

PyObject* TxRegEx::setNameSet(TxRegEx* self, PyObject *args, PyObject *kwds){
	char *name; 
	PyObject* list;
	int keep=0;
	static char *kwlist[] = {"name", "s", "keep", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "sO|i", kwlist, &name, &list, &keep))
        return 0; 

	ReferLinkHeap* heap=self->RegContext->getContextNameHeap(name);
	if (!keep) heap->empty();
	if (heap->Total==0){
		long len=PyList_Size(list); 
		ReferData str;
		for (long i=0; i<len; i++){
			PyObject* item=PyList_GetItem(list, i);
			uni2str(item, &str);
			ReferLink* link=heap->add(&str, 0, heap->Total); 
		}
	}
	return PyLong_FromLong(heap->Total);
}
PyObject* TxRegEx::getExpr(TxRegEx* self, PyObject *args, PyObject *kwds){
	if (self->RegExprSentence && self->RegExprSentence->P){
		return str2uni(self->RegExprSentence->P);
	} else {
		Py_RETURN_NONE;
	}
}












