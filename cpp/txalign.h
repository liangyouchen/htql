#ifndef TX_ALIGNMENT_H_2013_07_12
#define TX_ALIGNMENT_H_2013_07_12

#include <Python.h>
#include "structmember.h"

#include "py2_4.h"

class HtTextAlign; 
class ReferData; 

class TxAlign {
public:
    PyObject_HEAD

	HtTextAlign* Align;

	double MIDRCost[4];
public:
	static void dealloc(TxAlign* self);
	static PyObject* alloc(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int init(TxAlign *self, PyObject *args, PyObject *kwds);
	//global alignment
	static PyObject* align(TxAlign* self, PyObject *args, PyObject *kwds);
	static PyObject* alignMin(TxAlign* self, PyObject *args, PyObject *kwds);
	static PyObject* alignText(TxAlign* self, PyObject *args, PyObject *kwds);
	// local alignment
	static PyObject* findStr(TxAlign* self, PyObject *args, PyObject *kwds);
	// multiple alignment
	static PyObject* cluster(TxAlign* self, PyObject *args, PyObject *kwds);
	static PyObject* getAlignment(TxAlign* self, PyObject *args, PyObject *kwds);

	int set_aligned_item(PyObject* list_result, PyObject* list_offset, long i, ReferData* result, long offset);
};

static PyMethodDef TxAlign_Methods[] = {
    {"align", (PyCFunction)TxAlign::align, METH_VARARGS|METH_KEYWORDS, "align two strings with dynamic algorithm"},
    {"minAlign", (PyCFunction)TxAlign::alignMin, METH_VARARGS|METH_KEYWORDS, "search a string from a list of strings to find the closest match"},
    {"find", (PyCFunction)TxAlign::findStr, METH_VARARGS|METH_KEYWORDS, "find substrings with dynamic algorithm"},
    {"alignText", (PyCFunction)TxAlign::alignText, METH_VARARGS|METH_KEYWORDS, "align two text/html documents"},
    {"cluster", (PyCFunction)TxAlign::cluster, METH_VARARGS|METH_KEYWORDS, "cluster text"},
    {"getAlignment", (PyCFunction)TxAlign::getAlignment, METH_NOARGS, "get alignment"},
    {0, 0, 0, 0}  /* Sentinel */
};


static PyTypeObject TxAlign_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "htql.Align",             /*tp_name*/
    sizeof(TxAlign),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/

    (destructor)TxAlign::dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/ //(getattrfunc) TxDict::getattr
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
    "dynamic alignment algorithm",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TxAlign_Methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TxAlign::init,      /* tp_init */
    0,                         /* tp_alloc */
    TxAlign::alloc,                 /* tp_new */

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

