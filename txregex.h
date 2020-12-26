#ifndef TX_REGEX_H_2012_06_17
#define TX_REGEX_H_2012_06_17

#include <Python.h>
#include "structmember.h"
#include "referdata.h"

//#ifdef Python24		//only for python2.4 and lower
#include "py2_4.h"
//#endif 

class ReferLinkHeap; 
class RegExParser;
class RegExToken;
class RegExKeeper; 
class ReferData;
class ReferSet; 

class TxRegEx {
public:
    PyObject_HEAD

	RegExParser* RegContext; 
	RegExToken* TextStart;

	RegExToken* RegExpr; 
	RegExParser* RegParser;
	ReferData* RegExprSentence;
	int IsRegList;
	int IsKeptRegEx; 
	int clearRegExpr();
	int useKeptRegEx(ReferData* tuple); 

	ReferLinkHeap* KeptText; //to keep text, tokens, and regular expressions passed through python
	RegExKeeper* KeptRegEx; 

public:
	static void dealloc(TxRegEx* self);
	static PyObject* alloc(PyTypeObject *type, PyObject *args, PyObject *kwds);
	static int init(TxRegEx *self, PyObject *args, PyObject *kwds);
	static PyObject* regSearchStr(TxRegEx* self, PyObject *args, PyObject *kwds);
	static PyObject* regSearchList(TxRegEx* self, PyObject *args, PyObject *kwds);
	static PyObject* setExprStr(TxRegEx* self, PyObject *args, PyObject *kwds);
	static PyObject* setExprList(TxRegEx* self, PyObject *args, PyObject *kwds);
	static PyObject* regExprSearch(TxRegEx* self, PyObject *args, PyObject *kwds);
	static PyObject* setNameSet(TxRegEx* self, PyObject *args, PyObject *kwds);
	static PyObject* getExpr(TxRegEx* self, PyObject *args, PyObject *kwds);

	//main entry functions
	int parseRegExpr(const char* sentence, int is_list=0, int case_sensitive=true, int keep=false, const char* name=0);
	PyObject* searchRegTextStr(const char*text, int overlapping, int useindex, int group, long limit=-1);
	PyObject* searchRegTextList(PyObject*text, int overlapping, int useindex, int group, long limit=-1);

	//general functions
	int tokenizeListText(PyObject* text, RegExToken* token_text, long limit=-1);
	PyObject* buildListResults(ReferLinkHeap* results, int useindex);
	PyObject* buildStrResults(ReferLinkHeap* results, int useindex);
	PyObject* buildListResultsSet(ReferSet* results, int useindex, int withgroup);
	PyObject* buildStrResultsSet(ReferSet* results, int useindex, int withgroup);
};

static PyMethodDef TxRegEx_Methods[] = {
    {"reSearchStr", (PyCFunction)TxRegEx::regSearchStr, METH_VARARGS|METH_KEYWORDS, "search regex string"},
    {"reSearchList", (PyCFunction)TxRegEx::regSearchList, METH_VARARGS|METH_KEYWORDS, "search regex list"},
    {"setExprStr", (PyCFunction)TxRegEx::setExprStr, METH_VARARGS|METH_KEYWORDS, "set regular expression for string search"},
    {"setExprList", (PyCFunction)TxRegEx::setExprList, METH_VARARGS|METH_KEYWORDS, "set regular expression for list search"},
    {"reExprSearch", (PyCFunction)TxRegEx::regExprSearch, METH_VARARGS|METH_KEYWORDS, "do the string or list search using the set regex"},
    {"setNameSet", (PyCFunction)TxRegEx::setNameSet, METH_VARARGS|METH_KEYWORDS, "set a list as variable"},
    {"getExpr", (PyCFunction)TxRegEx::getExpr, METH_VARARGS|METH_KEYWORDS, "set a list as variable"},
    {0, 0, 0, 0}  /* Sentinel */
};

static PyTypeObject TxRegEx_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "htql.RegEx",             /*tp_name*/
    sizeof(TxRegEx),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/

    (destructor)TxRegEx::dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/ //(getattrfunc) TxRegEx::getattr
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
    "regular expression functions",           /* tp_doc */
    0,		               /* tp_traverse */
    0,		               /* tp_clear */
    0,		               /* tp_richcompare */
    0,		               /* tp_weaklistoffset */
    0,		               /* tp_iter */
    0,		               /* tp_iternext */
    TxRegEx_Methods,             /* tp_methods */
    0,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)TxRegEx::init,      /* tp_init */
    0,                         /* tp_alloc */
    TxRegEx::alloc,                 /* tp_new */

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

