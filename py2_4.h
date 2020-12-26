#ifndef HTQL_PY2_4_H
#define HTQL_PY2_4_H


#ifdef Python24		//only for python2.4 and lower
	#define Py_ssize_t int
	#define lenfunc int(*)(PyObject*)
	#define ssizeargfunc PyObject*(*)(PyObject*, int)
	#define PyVarObject_HEAD_INIT(type, size) PyObject_HEAD_INIT(type) size,
#endif

#ifndef METH_KEYWORDS1
	#ifdef Python27
		#define METH_KEYWORDS1	METH_KEYWORDS
	#else
		#define METH_KEYWORDS1	METH_VARARGS|METH_KEYWORDS
	#endif
#endif



#endif


