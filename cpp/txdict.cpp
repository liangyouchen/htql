
#include "txdict.h"
#include "referlink.h"
#include "stroper.h"
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



void TxDict::dealloc(TxDict* self){
	if (self->dict) delete self->dict; 
	self->dict=0;
	self->deleteSpaces();

#ifdef Python27
    self->ob_type->tp_free((PyObject*)self);
#else
	Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

void TxDict::deleteSpaces(){
	if (Spaces){
		free(Spaces);
		Spaces=0; 
	}
}

PyObject* TxDict::alloc(PyTypeObject *type, PyObject *args, PyObject *kwds){
    TxDict *self;

    self = (TxDict *)type->tp_alloc(type, 0);
    if (self != NULL) {
		self->CaseSensitive=false; 
		self->Spaces=0;
        self->dict=new ReferLinkHeap;
		self->dict->setCaseSensitivity(false); 
		self->dict->setDuplication(false); 
		self->dict->setSortOrder(SORT_ORDER_KEY_STR_INC); 
    }

    return (PyObject *)self;
}

int TxDict::init(TxDict *self, PyObject *args, PyObject *kwds){
    PyObject *sourcedict=0;
    static char *kwlist[] = {"dict", "case", "spaces", NULL};
	self->CaseSensitive=false; 
	self->deleteSpaces(); 
	char* spaces=0; 

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|Ois", kwlist, &sourcedict, &self->CaseSensitive, &spaces ))
        return -1; 

    if (self != NULL && !self->dict) {
        self->dict=new ReferLinkHeap;
		self->dict->setCaseSensitivity(self->CaseSensitive); 
		self->dict->setDuplication(false); 
		self->dict->setSortOrder(SORT_ORDER_KEY_STR_INC); 
   }

    if (sourcedict){
		self->set_dict(sourcedict); 
	}

	if (spaces){
		self->Spaces = (char*) malloc(sizeof(char)*(strlen(spaces)+1));
		strcpy(self->Spaces, spaces);
	}

    return 0;
}
PyObject* TxDict::setDict(TxDict *self, PyObject *args, PyObject *kwds){
    PyObject *sourcedict=0;
	int to_reset=0; 
    static char *kwlist[] = {"dict", "reset", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "O|i", kwlist, &sourcedict, &to_reset))
        return 0; 

	self->set_dict(sourcedict, to_reset); 

    return PyLong_FromLong(self->dict->Total);
}

PyObject* TxDict::searchKeys(TxDict *self, PyObject *args, PyObject *kwds){
	char* sentence=0; 
    PyObject *sourcedict=0;
    static char *kwlist[] = {"sentence", "dict", "order", NULL};
	char* orderby=0; 

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s|Os", kwlist, &sentence, &sourcedict, &orderby))
        return 0; 

    if (sourcedict){
		self->set_dict(sourcedict, true); 
	}

	ReferLinkHeap results;
	results.setDuplication(true); 
	results.setCaseSensitivity(self->CaseSensitive); 
	int order=orderSOURCE; 
	if (orderby && !stricmp(orderby, "value")) {
		order=orderVALUE; 
		results.setSortOrder(SORT_ORDER_VAL_STR_INC );
	} else if (orderby && !stricmp(orderby, "key")) {
		order=orderKEY; 
		results.setSortOrder(SORT_ORDER_KEY_STR_INC );
	}else{
		results.setSortOrder(SORT_ORDER_NUM_INC);
	}
	self->search_keys(sentence, &results, order );

	PyObject* result=PyList_New(results.Total);
	long i=0, count=0;
	for (ReferLink* link=(ReferLink*) results.moveFirst(); link; link=(ReferLink*) results.moveNext()){
		PyObject* tuple=PyTuple_New(2);
		PyTuple_SetItem(tuple, 0, str2uni(link->Name.P));
		if (link->Value.L>=0){
			PyTuple_SetItem(tuple, 1, str2uni(link->Value.P));
		}else{
			PyTuple_SetItem(tuple, 1, PyLong_FromLong(link->Data));
		}
		PyList_SetItem(result, count++, tuple);
	}

    return result;
}

PyObject* TxDict::keys(TxDict *self, PyObject *args, PyObject *kwds){
	PyObject* result=PyList_New(self->dict?self->dict->Total:0);
	long count=0;
	if (self->dict){
		for (ReferLink* link=(ReferLink*) self->dict->moveFirst(); link; link=(ReferLink*) self->dict->moveNext()){
			PyList_SetItem(result, count++, str2uni(link->Name.P));
		}
	}
    return result;
}

Py_ssize_t TxDict::getLength(TxDict *self){
	return self->dict?self->dict->Total:0; 
}
PyObject* TxDict::getItem(TxDict *self, PyObject *key){
	if (is_pystr(key)){
		ReferData str;
		uni2str(key, &str); 
		ReferLink* link=self->dict->findName(&str);
		if (link) {
			if (link->Value.L>=0) return str2uni(link->Value.P); 
			else return PyLong_FromLong(link->Data); 
		}else return 0;
	}else
		return 0;
	
}
int TxDict::setItem(TxDict *self, PyObject *key, PyObject *v){
	if (is_pystr(key) ){
		ReferData key1;
		uni2str(key, &key1); 
		ReferLink* link=self->dict->findName(&key1);
		if (!link) link=self->dict->add(&key1, 0, 0); 

		if (is_pystr(v)){
			uni2str(v, &link->Value);
		}else{
			link->Data = PyLong_AsLong(v); 
			link->Value.reset(); 
			link->Value.L=-1; 
		}
		return 0;
	}else
		return -1;
}

int TxDict::set_dict(PyObject* sourcedict, int to_reset){
	if (!this) return 0; 

	int err=0; 
	if (to_reset){
		dict->empty();
	}

	if (PyMapping_Check(sourcedict)){
		PyObject*items=PyMapping_Items(sourcedict);
		if (items){
			set_list(items);
			Py_DECREF(items);
		}
	}else if (PyList_Check(sourcedict)){
		set_list(sourcedict);
	}

	return err;
}
int TxDict::set_list(PyObject* sourcelist, int to_reset){
	if (!this) return 0; 

	int err=0; 
	if (to_reset){
		dict->empty();
	}

	if (PyList_Check(sourcelist)){
		unsigned int size=PyList_Size(sourcelist);
		for (unsigned int i=0; i<size; i++) {
			ReferData name, value;
			long longval=0;
			get_key_value(sourcelist, i, &name, &value, &longval);
			dict->add(&name, &value, longval);
		}
	}
	return 0;
}
int TxDict::is_space(char c){
	if (Spaces){
		return strchr(Spaces, c)!=NULL;
	}else{
		return !tStrOp::isAN_(c);
	}
}

int TxDict::search_keys(char* sentence, ReferLinkHeap* results, int orderby){
	if (!sentence || !sentence[0]) return 0; 

	char* p=sentence; 
	ReferLink* first; 
	ReferLink search; 
	char keep=0; 
	long last_i=0; 
	while (p && *p){
		search.Name.Set(p, strlen(p), false); 
		first=(ReferLink*) dict->moveFirstLarger((char*) &search); 
		if (!first) first=(ReferLink*) dict->moveLast(); 
		else if (tStrOp::strNcmp(p, first->Name.P, first->Name.L, false)){
			first=(ReferLink*) dict->movePrevious();
		}
		if (first){
			//get the longest match
			ReferData firstname; 
			firstname.Set(first->Name.P, first->Name.L, false); 
			ReferData value;
			for (last_i=0; last_i<firstname.L; last_i++){
				if ((this->CaseSensitive && p[last_i]!=firstname.P[last_i] ) 
					|| (!this->CaseSensitive && toupper(p[last_i])!=toupper(firstname.P[last_i])) ) break; 
			}
			while (last_i>0){
				if (last_i>0){
					keep=p[last_i]; 
					p[last_i]=0;
					first=(ReferLink*) dict->findName(p);
					p[last_i]=keep;
				}
				if ((!p[last_i] || is_space(p[last_i])) && first && last_i==first->Name.L){
					if (orderby==orderSOURCE){
						if (first->Value.L>=0){
							value.Set(first->Value.P, first->Value.L, false); 
						}else{
							value.setLong(first->Data); 
						}
						results->add(&first->Name, &value, p-sentence);
					}else if (orderby==orderVALUE || orderby==orderKEY ){
						results->add(&first->Name, &first->Value, first->Data);
					}
					p+=first->Name.L; 
					break;
				}
				last_i--; 
				while (last_i>0 && !is_space(p[last_i]) ) last_i--;
			}
		}
		while (*p && !is_space(*p) ) p++;
		if (is_space(*p) ) p++;
	}
	return 0;
}








