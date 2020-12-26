#ifndef TX_NAIVE_BAYES_H_2014_08_23
#define TX_NAIVE_BAYES_H_2014_08_23

#include <Python.h>
#include "structmember.h"

#include "py2_4.h"

class ReferLinkHeap; 
class HtNaiveBayes;
class ReferSet;
class ReferData; 

class TxNaiveBayes {
public:
    PyObject_HEAD
public:
	HtNaiveBayes* NaiveBayes; 
	ReferLinkHeap* FeatureNames; 
	ReferSet* DataSchema;

	static void dealloc(TxNaiveBayes* self);
	static PyObject* alloc(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int init(TxNaiveBayes *self, PyObject *args, PyObject *kwds);
	static PyObject* fitData(TxNaiveBayes *self, PyObject *args, PyObject *kwds);
	static PyObject* predProba(TxNaiveBayes *self, PyObject *args, PyObject *kwds);
	static PyObject* dumpXml(TxNaiveBayes *self, PyObject *args, PyObject *kwds);
	static PyObject* loadXml(TxNaiveBayes *self, PyObject *args, PyObject *kwds);

	int reset();
	int fit_data(PyObject* data, PyObject* feature_names, const char* outcome_name, int to_print); 
	int pred_proba(PyObject* item, ReferLinkHeap* preds, int to_print); 
	int setItemTuple(ReferData* tuple, PyObject* item); 
};

static PyMethodDef TxNaiveBayes_Methods[] = {
    {"fit", (PyCFunction)TxNaiveBayes::fitData, METH_VARARGS | METH_KEYWORDS, "fit the data\ndata -- training data\nfeature_names -- features\noutcome_name -- outcome"},
    {"predProba", (PyCFunction)TxNaiveBayes::predProba, METH_VARARGS | METH_KEYWORDS, "pred the item\nitem -- test data item"},
    {"loadXML", (PyCFunction)TxNaiveBayes::loadXml, METH_VARARGS | METH_KEYWORDS, "load classifier from XML that is produced by dumpXML()"},
    {"dumpXML", (PyCFunction)TxNaiveBayes::dumpXml, METH_VARARGS | METH_KEYWORDS, "dump an XML presentation of the classifier"},
    {0, 0, 0, 0}  /* Sentinel */
};


static PyTypeObject TxNaiveBayes_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "htql.NaiveBayes",             /*tp_name*/
    sizeof(TxNaiveBayes),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/

    (destructor)TxNaiveBayes::dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/ //(getattrfunc) TxTrend::getattr
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/

    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,   /*tp_as_mapping*/

    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/  //(getattrofunc) Htql_getattro
    0,                         /*tp_setattro*/

    0,                         /*tp_as_buffer*/

	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Naive Bayes classifier",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TxNaiveBayes_Methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TxNaiveBayes::init,      /* tp_init */
    0,                         /* tp_alloc */
    TxNaiveBayes::alloc,                 /* tp_new */

#ifndef Python27
    0, /* tp_free: Low-level free-memory routine */
    0, /* tp_is_gc: For PyObject_IS_GC */
    0, /* tp_bases */
    0, /*tp_mro: method resolution order */
    0, /* tp_cache */
    0, /* tp_subclasses */
    0 /* tp_weaklist */
#endif
};


#endif

