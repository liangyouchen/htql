
#include "txnaivebayes.h"
#include "referlink.h"
#include "stroper.h"
#include "referset.h"
#include "pyhtqlmodule.h"
#include "HtNaiveBayes.h"

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



void TxNaiveBayes::dealloc(TxNaiveBayes* self){
	if (self->NaiveBayes) delete self->NaiveBayes; 
	self->NaiveBayes=0;
	if (self->FeatureNames) delete self->FeatureNames; 
	self->FeatureNames=0; 
	if (self->DataSchema) delete self->DataSchema; 
	self->DataSchema=0; 

#ifdef Python27
    self->ob_type->tp_free((PyObject*)self);
#else
	Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

PyObject* TxNaiveBayes::alloc(PyTypeObject *type, PyObject *args, PyObject *kwds){
    TxNaiveBayes *self;

    self = (TxNaiveBayes *)type->tp_alloc(type, 0);
    if (self != NULL) {
		self->NaiveBayes = new HtNaiveBayes(); 
		self->FeatureNames = new ReferLinkHeap(); 
		self->FeatureNames->setSortOrder(SORT_ORDER_NUM_INC);
		self->DataSchema = new ReferSet(); 
    }

    return (PyObject *)self;
}

int TxNaiveBayes::init(TxNaiveBayes *self, PyObject *args, PyObject *kwds){
    if (self != NULL && !self->NaiveBayes) {
		self->NaiveBayes = new HtNaiveBayes(); 
		self->FeatureNames = new ReferLinkHeap(); 
		self->FeatureNames->setSortOrder(SORT_ORDER_NUM_INC);
		self->DataSchema = new ReferSet(); 
	}
    return 0;
}
int TxNaiveBayes::reset(){
	NaiveBayes->reset();
	FeatureNames->empty();
	DataSchema->reset();
	return 0;
}

PyObject* TxNaiveBayes::fitData(TxNaiveBayes *self, PyObject *args, PyObject *kwds){
    PyObject *data=0;
    PyObject *feature_names=0;
	char* outcome_name=0; 
	int to_print=0;

    static char *kwlist[] = {"data", "feature_names", "outcome_name", "to_print", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "OOs|i", kwlist, &data, &feature_names, &outcome_name, &to_print))
        return 0; 

	int err=self->fit_data(data, feature_names, outcome_name, to_print); 

    return PyLong_FromLong(err);
}

PyObject* TxNaiveBayes::predProba(TxNaiveBayes *self, PyObject *args, PyObject *kwds){
    PyObject *item=0;
	int to_print=0;

    static char *kwlist[] = {"item", "to_print", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &item, &to_print))
        return 0; 

	ReferLinkHeap preds; 
	self->pred_proba(item, &preds, to_print); 

	ReferLink* link=(ReferLink*) self->FeatureNames->moveLast();
	int isstr_label = !link->Value.P || link->Value.P[0]!='N';

	PyObject* predprobs=PyDict_New();
	for (link=(ReferLink*) preds.moveFirst(); link; link=(ReferLink*) preds.moveNext()){
		double val=*((double*) link->Value.P); 
		if (isstr_label){
			PyDict_SetItemString(predprobs, link->Name.P, PyFloat_FromDouble(val) );
		}else{
			PyDict_SetItem(predprobs, PyLong_FromLong(link->Name.getLong() ), PyFloat_FromDouble(val) );
		}
	}

    return predprobs;
}
PyObject* TxNaiveBayes::dumpXml(TxNaiveBayes *self, PyObject *args, PyObject *kwds){
	ReferData data;
	if (self->NaiveBayes){
		self->NaiveBayes->dumpXML(&data);
	}
	if (data.L) return str2uni(data.P); 
	Py_RETURN_NONE; 
}
PyObject* TxNaiveBayes::loadXml(TxNaiveBayes *self, PyObject *args, PyObject *kwds){
	char* xml=0;

    static char *kwlist[] = {"data", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &xml))
        return 0; 

	ReferData data;
	if (xml){ 
		data.Set(xml, strlen(xml), false);
	}
	int ret=self->NaiveBayes->loadXML(&data); 
	return PyLong_FromLong(ret); 
}



int TxNaiveBayes::fit_data(PyObject* data, PyObject* feature_names, const char* outcome_name, int to_print){
	long len, i; 
	ReferData str;
	ReferLink* link; 

	reset();

	//get feature names
	len=PyList_Size(feature_names); 
	for (i=0; i<len; i++){
		PyObject* item=PyList_GetItem(feature_names, i);
		uni2str(item, &str);
		link=FeatureNames->add(&str, 0, FeatureNames->Total); 
	}
	str=outcome_name;
	link=FeatureNames->add(&str, 0, FeatureNames->Total); 
	
	//prepare dataset
	ReferSet data_set; 
	data_set.setFieldsNum(FeatureNames->Total); 
	for (link=(ReferLink*) FeatureNames->moveFirst(); link; link=(ReferLink*) FeatureNames->moveNext()){
		data_set.setFieldName(link->Data, link->Name.P); 
	}
	DataSchema->reset();
	DataSchema->copyStruct(&data_set); 
	
	//get dataset
	len=PyList_Size(data); 
	for (i=0; i<len; i++){
		PyObject* item=PyList_GetItem(data, i);
		ReferData* tuple = data_set.newTuple(); 

		setItemTuple(tuple, item); 
	}

	ReferData outcome; 
	outcome=outcome_name; 
	ReferLinkHeap features; 
	for (link=FeatureNames->getReferLinkHead(); link; link=link->Next){
		if (!link->Name.Cmp(&outcome))
			features.add(&link->Name, &link->Value, link->Data);
	}
	NaiveBayes->fitData(&data_set, &features, &outcome); 
	return 0; 
}
int TxNaiveBayes::setItemTuple(ReferData* tuple, PyObject* item){	
	for (ReferLink* link=(ReferLink*) FeatureNames->moveFirst(); link; link=(ReferLink*) FeatureNames->moveNext()){
		PyObject* item1=0; 
		if (PyList_Check(item)){
			item1=PyList_GetItem(item, link->Data);
		}else if (PyDict_Check(item)){
			item1=PyDict_GetItemString(item, link->Name.P); 
		}
		if (!item1) continue; 
		ReferData value;
		int is_str=is_pystr(item1); 
		if (is_str){
			uni2str(item1, &value); 
		}else if (PyFloat_Check(item1)){
			double val = PyFloat_AsDouble(item1);
			value.setDouble(val);
		}else{
			long val = PyLong_AsLong(item1);
			value.setLong(val);
		//}else{
		//	return PyErr_Format(PyExc_RuntimeError, "Data type not supported: %s!", link->Name.P);
		}
		if (!is_str && !link->Value.P){
			link->Value = "N"; 
		}
		tuple[link->Data]=value.P; 
	}
	return 0;
}

int TxNaiveBayes::pred_proba(PyObject* item, ReferLinkHeap* preds, int to_print){
	ReferData* tuple = DataSchema->getQueryTuple();
	setItemTuple(tuple, item); 
	
	NaiveBayes->predictProba(tuple, preds, to_print);
	return 0;
}










