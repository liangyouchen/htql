#include "pyhtql.h"
#include "pybrowser.h"
#include "pyhtqltools.h"
#include "txdict.h"
#include "txregex.h"
#include "txalign.h"
#include "txnaivebayes.h"
#include "pyhtqlmodule.h"

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

//  get x64 builds working, install VS update: http://support.microsoft.com/kb/2519277
// http://sourceforge.net/p/sevenzip/discussion/45797/thread/02fd5870/


PyObject* ModuleInstance=0; 

int debug_str(char* str, int is_new_file){
	ReferData s; 
	s.Set(str, strlen(str), false); 
	s+="\r\n"; 
	if (is_new_file){
		s.saveFile("d:\\pyhtql_debug.txt");
	}else{
		s.appendFile("d:\\pyhtql_debug.txt");
	}
	return 0;
}
int debug_str(long data, char* str, int is_new_file){
	char buf[256];
	sprintf(buf, "%ld", data);
	ReferData s; 
	s.Set(str, strlen(str), false);
	s+=": "; s+=buf; 
	s+="\r\n"; 
	if (is_new_file){
		s.saveFile("d:\\pyhtql_debug.txt");
	}else{
		s.appendFile("d:\\pyhtql_debug.txt");
	}
	return 0;
}

int is_pystr(PyObject*strobj){
#ifdef Python27
	return PyString_Check(strobj);
#else
	return PyUnicode_Check(strobj);
#endif
}

int get_key_value(PyObject*items, unsigned int i, ReferData* name, ReferData* value, long* longval){
	PyObject* pair = PySequence_GetItem(items,i);
	PyObject* key = PySequence_GetItem(pair,0);
	PyObject* o = PySequence_GetItem(pair,1);

	if (longval) *longval=0; 
	uni2str(key, name); 
	if (is_pystr(o)){
		uni2str(o, value); 
	}else if (longval) {
		value->reset();
		value->L=-1;
		*longval=PyLong_AsLong(o); 
	}else{
		value->reset();
	}

	Py_DECREF(key);
	Py_DECREF(o);
	Py_DECREF(pair);
	return 0;
}

char* uni2str(PyObject*uniobj, ReferData* str){
#ifdef Python27
	char*p=PyString_AsString(uniobj);
	str->Set(p, p?strlen(p):0, false);
#else
	PyObject* askey=PyUnicode_AsASCIIString(uniobj);
	*str=PyBytes_AsString(askey); 
	Py_XDECREF(askey);
#endif
	return str->P;
}

PyObject* str2uni(const char* str){
	if (!str) str=""; 
#ifdef Python27
	return PyString_FromString(str ); 
#else
	return PyUnicode_FromString(str); //Py_BuildValue("s", str ); 
#endif	
}

static PyObject* module_query(PyObject *self, PyObject *args, PyObject *kwds)
{
    //PyObject* instance = PyObject_CallObject((PyObject*) &PyHtql_Type, 0); 
	PyObject* instance = PyObject_GetAttrString(ModuleInstance, "Htql"); 
	if (!instance) return 0;

	PyObject* results=PyHtql::query((PyHtql*) instance, args, kwds);
	Py_XDECREF(instance); 
	return results; 
}
static PyObject* module_results(PyObject *self, PyObject *args, PyObject *kwds)
{
    //PyObject* instance = PyObject_CallObject((PyObject*) &PyHtql_Type, 0); 
	PyObject* instance = PyObject_GetAttrString(ModuleInstance, "Htql"); 
	if (!instance) return 0;

	PyObject* results=PyHtql::get_results((PyHtql*) instance, args, kwds);
	Py_XDECREF(instance); 
	return results; 
}

static PyMethodDef module_methods[] = {
	{"query", (PyCFunction) module_query, METH_VARARGS | METH_KEYWORDS, "htql query"}, 
	{"results", (PyCFunction) module_results, METH_VARARGS | METH_KEYWORDS, "htql query results"}, 
    {0, 0, 0, 0}  /* Sentinel */
};


#ifndef Python27
static struct PyModuleDef htqlmodule = {
   PyModuleDef_HEAD_INIT,
   "htql",   /* name of module */
   "htql module that creates an extension type.", /* module documentation, may be NULL */
   -1,       /* size of per-interpreter state of the module,
                or -1 if the module keeps state in global variables. */
   module_methods
};
#endif


#ifndef PyMODINIT_FUNC	/* declarations for DLL import/export */
#ifdef Python27
#define PyMODINIT_FUNC void
#else
#define PyMODINIT_FUNC PyObject*
#endif
#endif

#ifdef Python27
PyMODINIT_FUNC inithtql(void) {
#else
PyMODINIT_FUNC PyInit_htql(void) {
#endif

    PyObject* m;

    if (PyType_Ready(&PyHtql_Type) < 0)
#ifdef Python27
        return;
#else
        return 0;
#endif
    if (PyType_Ready(&PyBrowser_Type) < 0)
#ifdef Python27
        return;
#else
        return 0;
#endif

    if (PyType_Ready(&PyTools_Type) < 0)
#ifdef Python27
        return;
#else
        return 0;
#endif

    if (PyType_Ready(&TxDict_Type) < 0)
#ifdef Python27
        return;
#else
        return 0;
#endif

    if (PyType_Ready(&TxRegEx_Type) < 0)
#ifdef Python27
        return;
#else
        return 0;
#endif

    if (PyType_Ready(&TxAlign_Type) < 0)
#ifdef Python27
        return;
#else
        return 0;
#endif

    if (PyType_Ready(&TxNaiveBayes_Type) < 0)
#ifdef Python27
        return;
#else
        return 0;
#endif



#ifdef Python27
    m = Py_InitModule3("htql", module_methods,
                       "htql module that creates an extension type.");
#else
    m = PyModule_Create(&htqlmodule);
#endif
	ModuleInstance = m; 
    
	if (m == NULL) 
#ifdef Python27
        return;
#else
        return 0;
#endif

	//to add each object, use PyType_Ready(Object_Type) above first
    Py_INCREF(&PyHtql_Type);
    PyModule_AddObject(m, "HTQL", (PyObject *)&PyHtql_Type);
    Py_INCREF(&PyBrowser_Type);
    PyModule_AddObject(m, "Browser", (PyObject *)&PyBrowser_Type);
    Py_INCREF(&PyTools_Type);
    PyModule_AddObject(m, "Tools", (PyObject *)&PyTools_Type);
    Py_INCREF(&TxDict_Type);
    PyModule_AddObject(m, "Dict", (PyObject *)&TxDict_Type);
    Py_INCREF(&TxRegEx_Type);
    PyModule_AddObject(m, "RegEx", (PyObject *)&TxRegEx_Type);
    Py_INCREF(&TxAlign_Type);
    PyModule_AddObject(m, "Align", (PyObject *)&TxAlign_Type);
    Py_INCREF(&TxNaiveBayes_Type);
    PyModule_AddObject(m, "NaiveBayes", (PyObject *)&TxNaiveBayes_Type);

    PyObject* instance = PyObject_CallObject((PyObject*) &PyHtql_Type, 0); 
	if (instance) {
		PyModule_AddObject(m, "Htql", instance); 
	}

#ifdef Python27
        return;
#else
        return m;
#endif
}

/* testing: 
import htql; b=htql.HTQL('<a>1</a><a>2</a><a>3</a>', '<a>:tx,tn'); 
for c1,c2 in b: print c1,c2;

  */
