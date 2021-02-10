#ifndef TX_MATCHER_H_2012_03_07
#define TX_MATCHER_H_2012_03_07

#include <Python.h>
#include "structmember.h"

#include "py2_4.h"

class ReferLinkHeap; 


class TxDict {
public:
    PyObject_HEAD
    ReferLinkHeap* dict;
	int CaseSensitive; 
	char* Spaces;
	void deleteSpaces(); 

	enum {orderSOURCE, orderKEY, orderVALUE}; 
public:
	static void dealloc(TxDict* self);
	static PyObject* alloc(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int init(TxDict *self, PyObject *args, PyObject *kwds);
	static PyObject* setDict(TxDict *self, PyObject *args, PyObject *kwds);
	static PyObject* searchKeys(TxDict *self, PyObject *args, PyObject *kwds);
	static PyObject* keys(TxDict *self, PyObject *args, PyObject *kwds);

	static Py_ssize_t getLength(TxDict *self);
	static PyObject* getItem(TxDict *self, PyObject *key);
	static int setItem(TxDict *self, PyObject *key, PyObject *v);

	int set_dict(PyObject* sourcedict, int to_reset=false); 
	int set_list(PyObject* sourcelist, int to_reset=false); 
	int is_space(char c); 
	int search_keys(char* sentence, ReferLinkHeap* results, int orderby=orderSOURCE);
};

static PyMethodDef TxDict_Methods[] = {
    {"setDict", (PyCFunction)TxDict::setDict, METH_VARARGS | METH_KEYWORDS, "set dictionary\r\ndict -- dictionary or (key, value) list\r\n[reset] -- to reset dictionary"},
    {"searchKeys", (PyCFunction)TxDict::searchKeys, METH_VARARGS | METH_KEYWORDS, "search keys from sentence\r\nsentence -- a sentence\r\ndict -- dictionary or (key, value) list"},
    {"keys", (PyCFunction)TxDict::keys, METH_NOARGS, "list all keys"},
    {0, 0, 0, 0}  /* Sentinel */
};

static PyMappingMethods TxDict_Mapping_Methods = {
	(lenfunc) TxDict::getLength,  //mp_length 
	(binaryfunc) TxDict::getItem,  //mp_subscript  
	(objobjargproc) TxDict::setItem,  //mp_ass_subscript 
};

static PyTypeObject TxDict_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "htql.Dict",             /*tp_name*/
    sizeof(TxDict),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/

    (destructor)TxDict::dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/ //(getattrfunc) TxDict::getattr
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/

    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    &TxDict_Mapping_Methods,   /*tp_as_mapping*/

    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/  //(getattrofunc) Htql_getattro
    0,                         /*tp_setattro*/

    0,                         /*tp_as_buffer*/

	Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "matching text to keywords",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TxDict_Methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TxDict::init,      /* tp_init */
    0,                         /* tp_alloc */
    TxDict::alloc,                 /* tp_new */

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

