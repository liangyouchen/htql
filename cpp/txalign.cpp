#include "txalign.h"
#include "referlink.h"
#include "stroper.h"
#include "pyhtqlmodule.h"
#include "alignment.h"
#include "HtTextAlign.h"
#include "htmlql.h"

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



void TxAlign::dealloc(TxAlign* self){
	if (self->Align) delete self->Align;
	self->Align=0; 

#ifdef Python27
    self->ob_type->tp_free((PyObject*)self);
#else
	Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

PyObject* TxAlign::alloc(PyTypeObject *type, PyObject *args, PyObject *kwds){
    TxAlign *self;

    self = (TxAlign *)type->tp_alloc(type, 0);
	if (self){
		self->MIDRCost[0]=0.0; 
		self->MIDRCost[1]=1.0; 
		self->MIDRCost[2]=1.0; 
		self->MIDRCost[3]=2.0; 
		self->Align=new HtTextAlign; 
	}
 
    return (PyObject *)self;
}

int TxAlign::init(TxAlign *self, PyObject *args, PyObject *kwds){
	if (self){
		self->MIDRCost[0]=0.0; 
		self->MIDRCost[1]=1.0; 
		self->MIDRCost[2]=1.0; 
		self->MIDRCost[3]=2.0; 
	}

    return 0;
}



PyObject* TxAlign::align(TxAlign* self, PyObject *args, PyObject *kwds){
	char* str1=0; 
	char* str2=0; 
 	int case_sensitive=true;
	static char *kwlist[] = {"str1", "str2", "case", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "ss|i", kwlist, &str1, &str2, &case_sensitive))
        return 0; 
	
	StrAlignment str_align; 
	str_align.IsCaseSensitive = case_sensitive;
	str_align.setIndelCost(self->MIDRCost[0], self->MIDRCost[1], self->MIDRCost[2], self->MIDRCost[3]); 
	str_align.MatchMismatchPenalty=0.0; 
	double cost=0.0; 
	long result_len=0;
	char* result_str1=0, *result_str2=0; 
	str_align.CompareStrings(str1, str2, &cost, &result_len, &result_str1, &result_str2); 

	PyObject* result=PyTuple_New(2);
	PyTuple_SetItem(result, 0, PyFloat_FromDouble(cost) ); 
	PyObject* pair=PyList_New(2); 
	PyList_SetItem(pair, 0, str2uni(result_str1)); 
	PyList_SetItem(pair, 1, str2uni(result_str2)); 
	PyTuple_SetItem(result, 1, pair); 

	if (result_str1) free(result_str1); 
	if (result_str2) free(result_str2); 
	return result;
}
PyObject* TxAlign::alignMin(TxAlign* self, PyObject *args, PyObject *kwds){
	char* str1=0; 
	PyObject* list2=0; 
 	int case_sensitive=true;
	int get_alignment=false; 
	static char *kwlist[] = {"str1", "list2", "case", "get_align",  NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "sO|ii", kwlist, &str1, &list2, &case_sensitive, &get_alignment))
        return 0; 
	
	StrAlignment str_align; 
	str_align.IsCaseSensitive = case_sensitive;
	str_align.setIndelCost(self->MIDRCost[0], self->MIDRCost[1], self->MIDRCost[2], self->MIDRCost[3]); 
	str_align.MatchMismatchPenalty=0.0; 
	double min_cost=10.0; 
	char* result_str1=0, *result_str2=0; 
	long result_len=0;
	long result_i=0; 

	double cost=0.0; 
	long align_len=0;
	char* align_str1=0, *align_str2=0; 
	long list_len=PyList_Size(list2); 
	ReferData str2;
	for (long i=0; i<list_len; i++){
		PyObject* str_elm=PyList_GetItem(list2, i);
		uni2str(str_elm, &str2); 
		str_align.CompareStrings(str1, str2.P, &cost, &align_len, get_alignment?&align_str1:0, get_alignment?&align_str2:0);
		cost/=(2*align_len); 
		if (cost<min_cost){
			min_cost=cost; 
			result_len=align_len;
			if (get_alignment){
				result_str1=align_str1; 
				result_str2=align_str2;
			}
			result_i=i;
		}else{
			if (get_alignment){
				if (align_str1) free(align_str1); 
				if (align_str2) free(align_str2); 
				align_str1=0;
				align_str2=0;
			}
		}
	}

	PyObject* result=PyTuple_New(get_alignment?3:2);
	PyTuple_SetItem(result, 0, PyLong_FromLong(result_i) ); 
	PyTuple_SetItem(result, 1, PyFloat_FromDouble(min_cost) ); 
	if (get_alignment){
		PyObject* pair=PyList_New(2); 
		PyList_SetItem(pair, 0, str2uni(result_str1)); 
		PyList_SetItem(pair, 1, str2uni(result_str2)); 
		PyTuple_SetItem(result, 2, pair); 

		if (result_str1) free(result_str1); 
		if (result_str2) free(result_str2); 
	}
	return result;
}
PyObject* TxAlign::findStr(TxAlign* self, PyObject *args, PyObject *kwds){
	char* str1=0; 
	char* str2=0; 
 	int case_sensitive=true;
	int whole_x=false; 
	static char *kwlist[] = {"str1", "str2", "case", "whole_x", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "ss|ii", kwlist, &str1, &str2, &case_sensitive, &whole_x))
        return 0; 
	
	HyperTagsAlignment str_find; 
	str_find.IsCaseSensitive = case_sensitive;
	str_find.setAlignType(HyperTagsAlignment::TYPE_CHAR);
	ReferData rstr1, rstr2; 
	rstr1.Set(str1, strlen(str1), false); 
	rstr2.Set(str2, strlen(str2), false); 
	str_find.setSouceData(&rstr1);
	str_find.setStrData(&rstr2);
	ReferData align_results; 
	double align_score=0.0;
	str_find.alignHyperTagsText(&align_results,  &align_score, whole_x); 
	HtmlQL ql; 
	ql.setSourceData(align_results.P, align_results.L, false);
	ql.setQuery("<position>: source_pos, str_pos, cost, from_pos, from_str_pos");
	long total=ql.getTuplesCount();
	ReferData source_pos, str_pos, cost, from_pos, from_str_pos;
	long isource_pos=0, istr_pos=0, ifrom_pos=0, ifrom_str_pos=0;
	double dcost=0;
	ReferData value; 
	long i=0;
	PyObject* result=PyList_New(total);
	for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
		PyObject* dict=PyDict_New(); 
		source_pos=ql.getValue("source_pos"); 
		str_pos=ql.getValue("str_pos"); 
		cost=ql.getValue("cost"); 
		from_pos=ql.getValue("from_pos"); 
		from_str_pos=ql.getValue("from_str_pos"); 

		isource_pos=source_pos.getLong();
		istr_pos=str_pos.getLong();
		dcost=cost.getDouble();
		ifrom_pos=from_pos.getLong();
		ifrom_str_pos=from_str_pos.getLong(); 

		value.Set(str1+ifrom_pos, isource_pos-ifrom_pos+1, true ); 
		PyDict_SetItemString(dict, "X", str2uni(value.P  ));
		value.Set(str2+ifrom_str_pos, istr_pos-ifrom_str_pos+1, true ); 
		PyDict_SetItemString(dict, "Y", str2uni(value.P ));
		PyDict_SetItemString(dict, "X_start", PyLong_FromLong(ifrom_pos));
		PyDict_SetItemString(dict, "Y_start", PyLong_FromLong(ifrom_str_pos));
		PyDict_SetItemString(dict, "X_to", PyLong_FromLong(isource_pos));
		PyDict_SetItemString(dict, "Y_to", PyLong_FromLong(istr_pos ));
		PyDict_SetItemString(dict, "score", PyFloat_FromDouble(dcost));
		PyList_SetItem(result, i++, dict); 
	}

	return result;	
}

PyObject* TxAlign::alignText(TxAlign* self, PyObject *args, PyObject *kwds){
	char* page1=0, *page2=0;
	char* text_type=0;
	static char *kwlist[] = {"page1", "page2", "text_type", NULL};
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "ss|s", kwlist, &page1, &page2, &text_type))
        return 0; 

	ReferData page1s, page2s; 
	page1s.Set(page1, strlen(page1), false); 
	page2s.Set(page2, strlen(page2), false); 
	HtTextAlign align; 
	if (text_type && !stricmp(text_type, "html")){
		align.setHtmlText(1, &page1s); 
		align.setHtmlText(2, &page2s); 
	}else if (text_type && !stricmp(text_type, "str")){
		align.setCharText(1, &page1s); 
		align.setCharText(2, &page2s); 
	}else{
		align.setPlainText(1, &page1s); 
		align.setPlainText(2, &page2s); 
	}
	double cost=0; 
	long result_len=0; 
	ReferLink** source_align_links=0, **str_align_link=0; 
	align.align(1,2,&cost, &result_len, &source_align_links, &str_align_link); 

	PyObject* result1=PyList_New(result_len);
	PyObject* result2=PyList_New(result_len);
	PyObject* offset1=PyList_New(result_len);
	PyObject* offset2=PyList_New(result_len);

	int cmpcode=0; 
	long i; 
	char* last_p1=page1;
	char* last_p2=page2; 
	ReferData empty; 
	empty="";
	for (i=0; i<result_len; ){
		cmpcode=align.cmpLinks(source_align_links[i], str_align_link[i]);
		if (cmpcode==0){  //match
			self->set_aligned_item(result1, offset1, i, &source_align_links[i]->Value, source_align_links[i]->Value.P-page1);  
			last_p1=source_align_links[i]->Value.P+source_align_links[i]->Value.L;

			self->set_aligned_item(result2, offset2, i, &str_align_link[i]->Value, str_align_link[i]->Value.P-page2);  
			last_p2=str_align_link[i]->Value.P+str_align_link[i]->Value.L;
			i++; 
		}else if (cmpcode==3){ //replace
			while (i<result_len && cmpcode==3){
				self->set_aligned_item(result1, offset1, i, &source_align_links[i]->Value, source_align_links[i]->Value.P-page1);  
				last_p1=source_align_links[i]->Value.P+source_align_links[i]->Value.L;
				
				self->set_aligned_item(result2, offset2, i, &str_align_link[i]->Value, str_align_link[i]->Value.P-page2);  
				last_p2=str_align_link[i]->Value.P+str_align_link[i]->Value.L;
				i++; 
				if (i>=result_len) break; 
				cmpcode=align.cmpLinks(source_align_links[i], str_align_link[i]);
			}
		}else if (cmpcode==2){ //link1 has data
			while (i<result_len && cmpcode==2){
				self->set_aligned_item(result1, offset1, i, &source_align_links[i]->Value, source_align_links[i]->Value.P-page1);  
				last_p1=source_align_links[i]->Value.P+source_align_links[i]->Value.L;

				self->set_aligned_item(result2, offset2, i, &empty, last_p2-page2);  
				i++;
				if (i>=result_len) break; 
				cmpcode=align.cmpLinks(source_align_links[i], str_align_link[i]);
			}
		}else if (cmpcode==1){ //link2 has data	
			while (i<result_len && cmpcode==1){
				self->set_aligned_item(result1, offset1, i, &empty, last_p1-page1);  

				self->set_aligned_item(result2, offset2, i, &str_align_link[i]->Value, str_align_link[i]->Value.P-page2);  
				last_p2=str_align_link[i]->Value.P+str_align_link[i]->Value.L;

				i++; 
				if (i>=result_len) break; 
				cmpcode=align.cmpLinks(source_align_links[i], str_align_link[i]);
			}
		}
	}

	if (source_align_links) free(source_align_links); 
	if (str_align_link) free(str_align_link);
	

	PyObject* result_dict=PyDict_New();
	PyDict_SetItemString(result_dict, "X", result1 );
	PyDict_SetItemString(result_dict, "Y", result2 );
	PyDict_SetItemString(result_dict, "X_index", offset1 );
	PyDict_SetItemString(result_dict, "Y_index", offset2 );
	PyDict_SetItemString(result_dict, "L", PyLong_FromLong(result_len));
	PyDict_SetItemString(result_dict, "cost", PyFloat_FromDouble(cost));
	return result_dict;
}
int TxAlign::set_aligned_item(PyObject* list_result, PyObject* list_offset, long i, ReferData* result, long offset){
	ReferData str;
	str.Set(result->P, result->L, true); 
	PyList_SetItem(list_result, i, str2uni(str.P)); 
	PyObject* offset_item = PyTuple_New(2); 
	PyTuple_SetItem(offset_item, 0, PyLong_FromLong(offset)); 
	PyTuple_SetItem(offset_item, 1, PyLong_FromLong(str.L)); 
	PyList_SetItem(list_offset, i, offset_item); 
	return 0;
}

PyObject* TxAlign::cluster(TxAlign* self, PyObject *args, PyObject *kwds){
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

	self->Align->reset(); 

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
			self->Align->setHtmlText(i+1, &str);
		}else{
			self->Align->setPlainText(i+1, &str);
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
PyObject* TxAlign::getAlignment(TxAlign* self, PyObject *args, PyObject *kwds){
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


