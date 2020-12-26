#ifndef PY_HTQL_MODULE_H_CLY_2011_03_03
#define PY_HTQL_MODULE_H_CLY_2011_03_03

#include <Python.h>
#include "structmember.h"

class ReferData;

int is_pystr(PyObject*strobj); 
char* uni2str(PyObject*uniobj, ReferData* str);
PyObject* str2uni(const char* str);
int debug_str(char* str, int is_new_file=false);
int debug_str(long data, char* str, int is_new_file=false);
int get_key_value(PyObject*items, unsigned int i, ReferData* name, ReferData* value, long* longval=0);


#endif

