#ifndef PY_HTQL_H_CLY_2011_03_03
#define PY_HTQL_H_CLY_2011_03_03

#include <Python.h>
#include "structmember.h"

#include "py2_4.h"

class HTQL; 


class PyHtql {
public:
    PyObject_HEAD
    HTQL* ql;
	long count;

public:
	static void dealloc(PyHtql* self);
	static PyObject* alloc(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int init(PyHtql *self, PyObject *args, PyObject *kwds);
	static PyObject* iter(PyHtql *self);
	static PyObject* iternext(PyHtql *self);
	static PyObject* getattr(PyHtql *self, char* attrname);
	static PyObject* getattro(PyHtql *self, PyObject* attr);
	static Py_ssize_t getitem_size(PyHtql *self);
	static PyObject* getitem(PyHtql *self, PyObject* key);
	static PyObject* getsequence(PyHtql *self, Py_ssize_t i);
	static PyObject* query(PyHtql* self, PyObject *args, PyObject *kwds);
	static PyObject* get_results(PyHtql* self, PyObject *args, PyObject *kwds);
	static PyObject* fields(PyHtql* self, PyObject *args, PyObject *kwds);
	static PyObject* setvar(PyHtql* self, PyObject *args, PyObject *kwds);
};

static PyMethodDef PyHtql_Methods[] = {
    {"query", (PyCFunction)PyHtql::query, METH_VARARGS | METH_KEYWORDS, "make an htql query\r\nsource -- html source\r\nquery -- htql expression\r\n[url] -- source url"},
    {"results", (PyCFunction)PyHtql::get_results, METH_VARARGS | METH_KEYWORDS, "get htql query results"},
    {"set_var", (PyCFunction)PyHtql::setvar, METH_VARARGS | METH_KEYWORDS, "set global variable"},
    {"fields", (PyCFunction)PyHtql::fields, METH_NOARGS, "get fields"},
    {0, 0, 0, 0}  /* Sentinel */
};

static PyMappingMethods PyHtql_Mapping = {
	(lenfunc) PyHtql::getitem_size, /*mp_length*/
	(binaryfunc) PyHtql::getitem, /*mp_subscript*/
	0,  /*mp_ass_subscript*/
};
static PySequenceMethods PyHtql_Sequence = {
	(lenfunc ) PyHtql::getitem_size, /*sq_length */
	0, /*sq_concat */
	0,  /*sq_repeat*/
	(ssizeargfunc) PyHtql::getsequence,  /*sq_item*/
	0,  /*sq_ass_item*/
	0,  /*sq_contains*/
	0,  /*sq_inplace_concat*/
	0,  /*sq_inplace_repeat*/
};

static PyTypeObject PyHtql_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "htql.HTQL",             /*tp_name*/
    sizeof(PyHtql),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyHtql::dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/ //(getattrfunc) PyHtql::getattr
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    &PyHtql_Sequence,                         /*tp_as_sequence*/
    &PyHtql_Mapping,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/  //(getattrofunc) Htql_getattro
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
 #ifdef Python27
   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_ITER, /*tp_flags*/
#else
   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
#endif
    "create an HTQL objects; or with (source, query, [url]) to query a source html",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    (getiterfunc) PyHtql::iter,		               /* tp_iter */
    (iternextfunc) PyHtql::iternext,		               /* tp_iternext */
    PyHtql_Methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyHtql::init,      /* tp_init */
    0,                         /* tp_alloc */
    PyHtql::alloc,                 /* tp_new */

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

