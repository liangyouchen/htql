
#include "pyhtql.h"
#include "htmlql.h"

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



void PyHtql::dealloc(PyHtql* self){
	if (self->ql) delete self->ql; 
	self->ql=0;

#ifdef Python27
    self->ob_type->tp_free((PyObject*)self);
#else
	Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

PyObject* PyHtql::alloc(PyTypeObject *type, PyObject *args, PyObject *kwds){
    PyHtql *self;

    self = (PyHtql *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->ql=new HtmlQL;
		self->count=0;
    }

    return (PyObject *)self;
}

int PyHtql::init(PyHtql *self, PyObject *args, PyObject *kwds){
    char *source=0, *query=0, *url=0;
    static char *kwlist[] = {"source", "query", "url", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|sss", kwlist, 
                                      &source, &query, 
                                      &url))
        return -1; 

    if (self != NULL && !self->ql) {
        self->ql=new HtmlQL;
 		self->count=0;
   }

    if (source) self->ql->setSourceData(source, strlen(source), true); 
	if (url) self->ql->setSourceUrl(url, strlen(url) ); 
	if (query) {
		self->ql->setQuery(query); 
		self->count=0;
		if (!self->ql->isEOF())
			self->count++;
	}

    return 0;
}

PyObject* PyHtql::iter(PyHtql *self){
	if (self && self->ql){
		Py_INCREF(self);
		self->count=0;
		self->ql->moveFirst(); 
	}
	return (PyObject *)self;
}

PyObject* PyHtql::iternext(PyHtql *self){
	if (!self || !self->ql) return 0; 

	if (self->ql->isEOF()) {
		self->count=0;
		self->ql->moveFirst();
	}else if (self->count>0) {
		self->ql->moveNext(); 
	}

	if (self->ql->isEOF())
		return 0;
	else
		self->count++;

	int nfields=self->ql->getFieldsCount();
	
	PyObject* result=0;
	if (nfields==1){
		result=str2uni(self->ql->getValue(1));
	}else{
		result=PyTuple_New(nfields);
		for (long i=0; i<nfields; i++){
			PyTuple_SetItem(result, i, str2uni(self->ql->getValue(i+1)));
		}
	}

	/*char** varlist=(char**) malloc((nfields+1)*sizeof(char*)); 
	char* fmt=(char*) malloc(sizeof(char)*(nfields+1)); 
	for (int i=0; i<nfields; i++) {
		varlist[i]=self->ql->getValue(i+1);
		fmt[i]='s';
	}
	varlist[nfields]=0; 
	fmt[nfields]=0;

	PyObject *result = Py_VaBuildValue(fmt, (char*) varlist);

	if (varlist) free(varlist); 
	if (fmt) free(fmt); 
	*/
	return result;
}
PyObject* PyHtql::getattr(PyHtql *self, char* attrname){
	if (!self || !self->ql || self->ql->isEOF()) return 0;
	char* value=self->ql->getValue(attrname); 
	return str2uni(value);
}
PyObject* PyHtql::getattro(PyHtql *self, PyObject* attr){
	if (!self || !self->ql || self->ql->isEOF()) return 0;

#ifdef Python27
	return PyString_FromString(attr->ob_type->tp_name ); 
#else
	return Py_BuildValue("s", Py_TYPE(attr)->tp_name ); 
#endif
}
Py_ssize_t PyHtql::getitem_size(PyHtql *self){
	int size=0; 
	if (self && self->ql && !self->ql->isEOF()){
		size=self->ql->getFieldsCount(); 
	}
	return size;
}

PyObject* PyHtql::getitem(PyHtql *self, PyObject* key){
	if (!self || !self->ql || self->ql->isEOF()) return 0;
#ifdef Python27
	if (PyString_Check(key)){
		char* keyname=PyString_AsString(key); 
		char* value=self->ql->getValue(keyname); 
		if (value) return PyString_FromString(value); 
		else return 0;
	}else if (PyInt_Check(key)){
		long index=PyLong_AsLong(key); 
		char* value=0; 
		if (index>=0) value=self->ql->getValue(index+1); 
		else value=self->ql->getValue(self->ql->getFieldsCount() + index+1); 
		if (value) return PyString_FromString(value); 
		else return 0;
	}else
		return 0;
#else
	if (PyUnicode_Check(key)){
		PyObject* askey=PyUnicode_AsASCIIString(key);
		char* keyname=(char*)PyBytes_AsString(askey); 
		char* value=self->ql->getValue(keyname); 
		Py_XDECREF(askey);

		PyObject* result=0; 
		if (value) result=Py_BuildValue("s", value); 
		return result;
	}else if (PyLong_Check(key)){
		long index=PyLong_AsLong(key); 
		char* value=0; 
		if (index>=0) value=self->ql->getValue(index+1); 
		else value=self->ql->getValue(self->ql->getFieldsCount() + index+1); 
		if (value) return Py_BuildValue("s", value); 
		else return 0;
	}else
		return 0;
#endif
}

PyObject* PyHtql::getsequence(PyHtql *self, Py_ssize_t i){
	if (!self || !self->ql || self->ql->isEOF()) return 0;
	return Py_BuildValue("i", i);
	char* value=self->ql->getValue(i+1); 
	return str2uni(value);
}

PyObject* PyHtql::setvar(PyHtql* self, PyObject *args, PyObject *kwds){
	if (!self || !self->ql) return 0; 

    char *name=0, *value=0;
    static char *kwlist[] = {"name", "value", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "ss", kwlist, 
                                      &name, &value))
        return 0; 

	self->ql->setGlobalVariable(name, value); 

	return PyLong_FromLong(1);
}

PyObject* PyHtql::query(PyHtql* self, PyObject *args, PyObject *kwds){
	if (PyHtql::init(self, args, kwds)<0)
		return 0;

	long tuples=self->ql->getTuplesCount(); 
	long fields=self->ql->getFieldsCount(); 


	PyObject* result=PyList_New(tuples);
	long i=0, count=0;
	for (self->ql->moveFirst(); !self->ql->isEOF(); self->ql->moveNext()){
		PyObject* tuple=PyTuple_New(fields);
		for (i=0; i<fields; i++){
			PyTuple_SetItem(tuple, i, str2uni(self->ql->getValue(i+1)));
		}
		PyList_SetItem(result, count++, tuple);
	}
	

	/*
	char* fmt=(char*) malloc(sizeof(char)*(tuples*(fields+4)+4)); 
	char** varlist=(char**) malloc((tuples*(fields+4)+4)*sizeof(char*)); 

	strcpy(fmt, "[");
	long i=0, count=0;
	for (self->ql->moveFirst(); !self->ql->isEOF(); self->ql->moveNext()){
		strcat(fmt, "(");
		for (i=0; i<fields; i++){
			strcat(fmt, "s");
			varlist[count++]=self->ql->getValue(i+1);
		}
		strcat(fmt, ")");
	}
	strcat(fmt, "]");

	PyObject *result = Py_VaBuildValue(fmt, (char*) varlist);

	if (varlist) free(varlist); 
	if (fmt) free(fmt); 
	*/
	return result;
}

PyObject* PyHtql::get_results(PyHtql* self, PyObject *args, PyObject *kwds){
    long useindex=0;
    static char *kwlist[] = {"useindex", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|i", kwlist, 
                                      &useindex))
        return 0; 

	PyObject* result=0;
	if (self->ql){
		long tuples=self->ql->getTuplesCount(); 
		long fields=self->ql->getFieldsCount(); 

		result=PyList_New(tuples);
		long i=0, count=0;
		long position=0, offset=0; 
		for (self->ql->moveFirst(); !self->ql->isEOF(); self->ql->moveNext()){
			PyObject* tuple=PyTuple_New(fields);
			for (i=0; i<fields; i++){
				position=0; offset=0;
				self->ql->getFieldOffset(i+1, &position, &offset);
				if (useindex){
					PyObject* cell=PyTuple_New(2);
					PyTuple_SetItem(cell, 0, PyLong_FromLong(position));
					PyTuple_SetItem(cell, 1, PyLong_FromLong(offset));
					PyTuple_SetItem(tuple, i, cell);
				}else{
					PyTuple_SetItem(tuple, i, str2uni(self->ql->getValue(i+1)));
				}
			}
			PyList_SetItem(result, count++, tuple);
		}
	}
	return result;
}

PyObject* PyHtql::fields(PyHtql* self, PyObject *args, PyObject *kwds){
	if (!self || !self->ql) return 0; 

	int nfields=self->ql->getFieldsCount();

	PyObject* result=0;
	if (nfields==1){
		result=str2uni(self->ql->getFieldName(1));
	}else{
		result=PyTuple_New(nfields);
		for (long i=0; i<nfields; i++){
			PyTuple_SetItem(result, i, str2uni(self->ql->getFieldName(i+1)));
		}
	}

	/*
	char** varlist=(char**) malloc((nfields+1)*sizeof(char*)); 
	char* fmt=(char*) malloc(sizeof(char)*(nfields+1)); 
	for (int i=0; i<nfields; i++) {
		varlist[i]=self->ql->getFieldName(i+1);
		fmt[i]='s';
	}
	varlist[nfields]=0; 
	fmt[nfields]=0;

	PyObject *result = Py_VaBuildValue(fmt, (char*) varlist);

	if (varlist) free(varlist); 
	if (fmt) free(fmt); 
	*/
	return result;
}




