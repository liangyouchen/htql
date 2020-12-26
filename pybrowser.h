#ifndef PY_BROWSER_H_CLY_2011_03_03
#define PY_BROWSER_H_CLY_2011_03_03

#include <Python.h>
#include "structmember.h"
#include "referdata.h"

#include "py2_4.h"


class HtBrowser;

class PyBrowser {
public:
    PyObject_HEAD
    void* browser;
	HtBrowser* url_helper; 
	enum {PB_UNKNOWN, PB_SOCKBROWSER, PB_IRB_BROWSER }; 
	int browser_type; //0: nothing; 1: sock browser; 2: IRobot browser 
	int navigation_type; //0:goUrl; 1:goForm, form htql; 
	ReferData navigation_url;
	ReferData form_htql;
	ReferData input_htql;

public:
	static void dealloc(PyBrowser* self);
	static PyObject* alloc(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int init(PyBrowser *self, PyObject *args, PyObject *kwds);
	static PyObject * getattr(PyBrowser *self, char *name);
	static int setattr(PyBrowser *self, char *name, PyObject *v);
	static PyObject* goUrl(PyBrowser *self, PyObject *args, PyObject *kwds);
	static PyObject* goForm(PyBrowser *self, PyObject *args, PyObject *kwds);
	static PyObject* useFrame(PyBrowser *self, PyObject *args, PyObject *kwds);
	static PyObject* click(PyBrowser *self, PyObject *args, PyObject *kwds);
	int parseVarCookies(PyObject* values, PyObject* cookies, PyObject* headers); 
	static PyObject* setPage(PyBrowser* self, PyObject *args, PyObject *kwds);
	static PyObject* getPage(PyBrowser* self);
	static PyObject* getUpdatedPage(PyBrowser* self);
	static PyObject* runFunction(PyBrowser *self, PyObject *args, PyObject *kwds);
	static PyObject* connectBrowser(PyBrowser* self, PyObject *args, PyObject *kwds);
	static PyObject* setTimeout(PyBrowser* self, PyObject *args, PyObject *kwds);

	void delete_browser();
	void* new_browser(int type);
	int set_url(const char* url);
	int set_page(const char*page, const char* url); 
	int get_page(ReferData* page, ReferData* url); 
	int get_updated_page(ReferData* page, ReferData* url); 
	int set_form_variable(const char* name, const char* value); 
	int set_header_variable(const char* name, const char* value); 
	int set_cookie(const char* name, const char* value); 
	int use_form_query(const char* form_htql);
	int use_form_index(int form_index1);
	int click_item(const char* item_htql, int wait);
	int navigate();
};

static PyMethodDef PyBrowser_Methods[] = {
    {"goUrl", (PyCFunction)PyBrowser::goUrl, METH_VARARGS | METH_KEYWORDS,
     "Go to URL."},
    {"goForm", (PyCFunction)PyBrowser::goForm, METH_VARARGS | METH_KEYWORDS,
     "submit form"},
    {"useFrame", (PyCFunction)PyBrowser::useFrame, METH_VARARGS | METH_KEYWORDS,
     "use frame"},
    {"click", (PyCFunction)PyBrowser::click, METH_VARARGS | METH_KEYWORDS,
     "click item"},
    {"setPage", (PyCFunction)PyBrowser::setPage, METH_VARARGS | METH_KEYWORDS,
     "Set the current page."},
    {"getPage", (PyCFunction)PyBrowser::getPage, METH_NOARGS,
     "Return the current page."},
    {"getUpdatedPage", (PyCFunction)PyBrowser::getUpdatedPage, METH_NOARGS,
     "Return the current updated page."},
    {"runFunction", (PyCFunction)PyBrowser::runFunction, METH_VARARGS | METH_KEYWORDS,
     "Evaluate function."},
    {"connectBrowser", (PyCFunction)PyBrowser::connectBrowser, METH_VARARGS | METH_KEYWORDS,
     "connect to IrbBrowser"},
    {"setTimeout", (PyCFunction)PyBrowser::setTimeout, METH_VARARGS | METH_KEYWORDS,
     "set timeout for connect and transfer"},
    {NULL, NULL, 0, NULL}           /* sentinel */
};

static PyTypeObject PyBrowser_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "htql.Browser",             /*tp_name*/
    sizeof(PyBrowser),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/

	/* Methods to implement standard operations */
    (destructor)PyBrowser::dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/ //(getattrfunc)PyBrowser::getattr
    0,                         /*tp_setattr*/ //(setattrfunc)PyBrowser::setattr
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/

	/* Method suites for standard classes */
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/

	/* More standard operations (here for binary compatibility) */
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/  //(getattrofunc) Htql_getattro
    0,                         /*tp_setattro*/

	/* Functions to access object as input/output buffer */
    0,                         /*tp_as_buffer*/

	/* Flags to define presence of optional/expanded features */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "create a browser objects; or with (url) to go url",           /* tp_doc */

	/* call function for all accessible objects */
    0,		               /* tp_traverse */

	/* delete references to contained objects */
    0,		               /* tp_clear */

	/* rich comparisons */
    0,		               /* tp_richcompare */

	/* weak reference enabler */
    0,		               /* tp_weaklistoffset */

	/* Iterators */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */

	/* Attribute descriptor and subclassing stuff */
    PyBrowser_Methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)PyBrowser::init,      /* tp_init */
    0,                         /* tp_alloc */
    PyBrowser::alloc,                 /* tp_new */

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
