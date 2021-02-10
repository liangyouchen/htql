#ifndef PY_HTQL_TOOLS_H_CLY_2011_04_30
#define PY_HTQL_TOOLS_H_CLY_2011_04_30

#include <Python.h>
#include "structmember.h"

#include "py2_4.h"


class HtTextAlign; 

class PyTools {
public:
    PyObject_HEAD

	HtTextAlign* Align;

public:
	static void dealloc(PyTools* self);
	static PyObject* alloc(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int init(PyTools *self, PyObject *args, PyObject *kwds);
	static PyObject* cluster(PyTools* self, PyObject *args, PyObject *kwds);
	static PyObject* getAlignment(PyTools* self, PyObject *args, PyObject *kwds);
	static PyObject* regSearchStr(PyTools* self, PyObject *args, PyObject *kwds);
	static PyObject* regSearchList(PyTools* self, PyObject *args, PyObject *kwds);
	//static PyObject* regSearch(PyTools* self, PyObject *args, PyObject *kwds);
};

static PyMethodDef PyTools_Methods[] = {
    {"cluster", (PyCFunction)PyTools::cluster, METH_VARARGS | METH_KEYWORDS, "cluster text"},
    {"getAlignment", (PyCFunction)PyTools::getAlignment, METH_NOARGS, "get alignment"},
    {"reSearchStr", (PyCFunction)PyTools::regSearchStr, METH_VARARGS|METH_KEYWORDS, "search regex string"},
    {"reSearchList", (PyCFunction)PyTools::regSearchList, METH_VARARGS|METH_KEYWORDS, "search regex list"},
    {0, 0, 0, 0}  /* Sentinel */
};


static PyTypeObject PyTools_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "htql.Tools",             /*tp_name*/
    sizeof(PyTools),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)PyTools::dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/ //(getattrfunc) PyHtql::getattr
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/  //(getattrofunc) Htql_getattro
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
 #ifdef Python27
   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
#else
   Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
#endif
    "create a Tools object",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    PyTools_Methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyTools::init,      /* tp_init */
    0,                         /* tp_alloc */
    PyTools::alloc,                 /* tp_new */

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

