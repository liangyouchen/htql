#include "pybrowser.h"

#include "PathNavigationBrowser.h"
#include "IrbNavigationBrowser.h"
#include "htbrowser.h"

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



void PyBrowser::dealloc(PyBrowser* self){
	self->delete_browser(); 
	self->browser_type=PB_UNKNOWN;
	self->navigation_type=0;

#ifdef Python27
    self->ob_type->tp_free((PyObject*)self);
#else
	Py_TYPE(self)->tp_free((PyObject*)self);
#endif
}

PyObject* PyBrowser::alloc(PyTypeObject *type, PyObject *args, PyObject *kwds){
    PyBrowser *self;

    self = (PyBrowser *)type->tp_alloc(type, 0);
	self->browser=0;
	self->url_helper=0;
	self->browser_type=PB_UNKNOWN;

    return (PyObject *)self;
}
int PyBrowser::init(PyBrowser *self, PyObject *args, PyObject *kwds){
	int type=-1; 
	PyObject* otype=0; 
    static char *kwlist[] = {"type", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|O", kwlist,&otype))
        return -1; 

	if (!self) return -1; 

	if (otype){
#ifdef Python27
		if (PyString_Check(otype)){
			char* ctype=PyString_AsString(otype); 
			if (ctype && (!stricmp(ctype, "http") || !stricmp(ctype, "sock")) ){
				type=1;
			}else if (ctype && (!stricmp(ctype, "irb") || !stricmp(ctype, "irobot") ) ){
				type=2; 
			}
		}else if (PyInt_Check(otype)){
			type=PyLong_AsLong(otype); 
		}
#else
		if (PyUnicode_Check(otype)){
			ReferData ctype; 
			uni2str(otype, &ctype);
			if (!ctype.Cmp("http", 4, false) || !ctype.Cmp("sock", 4, false)){
				type=1;
			}else if (!ctype.Cmp("irb", 3, false) || !ctype.Cmp("irobot", 6, false) ){
				type=2; 
			}
		}else if (PyLong_Check(otype)){
			type=PyLong_AsLong(otype); 
		}
#endif
	}

	if (type>=0) self->new_browser(type);

    return 0;
}


PyObject* PyBrowser::goUrl(PyBrowser *self, PyObject *args, PyObject *kwds){
    char *url=0;
	PyObject* values=0;
	PyObject* cookies=0;
	PyObject* headers=0;
    static char *kwlist[] = {"url", "values", "cookies", "headers", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|sOOO", kwlist, &url, &values, &cookies, &headers))
        return 0; 

	if (!self) return 0; 

	int err=0;
	if (self->browser_type==PB_UNKNOWN) self->new_browser(PB_SOCKBROWSER); 

	if (url) err=self->set_url(url);
	if (err>=0) err=self->parseVarCookies(values, cookies, headers); 
	if (url && err>=0) err=self->navigate();

	return getPage(self); 
}
PyObject* PyBrowser::useFrame(PyBrowser *self, PyObject *args, PyObject *kwds){
	long index=0;
	char* htql=0; 
    static char *kwlist[] = {"index", "htql",  NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "i|s", kwlist, &index, &htql))
        return 0; 

	if (!self) return 0; 

	int err=0;
	if (self->browser_type==PB_SOCKBROWSER){
		// to do later ...
		//err=((HtBrowser*) self->browser)->(index, htql); 

	}else if (self->browser_type==PB_IRB_BROWSER){
		err=((PathNavigationBrowser*) self->browser)->useFrame(index, htql);
	}
	return Py_BuildValue("i", (char*) err);

}
PyObject* PyBrowser::goForm(PyBrowser *self, PyObject *args, PyObject *kwds){
	PyObject* form=0;
	PyObject* values=0;
	PyObject* cookies=0;
	PyObject* headers=0;
    static char *kwlist[] = {"form", "values", "cookies", "headers",  NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOO", kwlist, &form, &values, &cookies, &headers))
        return 0; 

	if (!self) return 0; 

	int err=0; 
	if (form){
#ifdef Python27
		if (PyString_Check(form)){
			char* query=PyString_AsString(form); 
			err=self->use_form_query(query); 
		}else if (PyInt_Check(form)){
			int index1=PyLong_AsLong(form); 
			err=self->use_form_index(index1); 
		}
#else
		if (PyUnicode_Check(form)){
			ReferData query; 
			uni2str(form, &query);
			err=self->use_form_query(query.P); 
		}else if (PyLong_Check(form)){
			int index1=PyLong_AsLong(form); 
			err=self->use_form_index(index1); 
		}
#endif
	}

	if (err>=0) err=self->parseVarCookies(values, cookies, headers); 
	if (err>=0) err=self->navigate();

	return getPage(self); 
}
PyObject* PyBrowser::click(PyBrowser *self, PyObject *args, PyObject *kwds){
	char* query=0;
	int wait=40; 
    static char *kwlist[] = {"query", "wait", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s|i", kwlist, &query, &wait))
        return 0; 

	if (!self) return 0; 

	int err=0; 
	if (query){
		err=self->click_item(query, wait); 
	}
	
	return getPage(self); 
}

int PyBrowser::parseVarCookies(PyObject* values, PyObject* cookies, PyObject* headers){
	int err=0;
	if (values){
		PyObject*items=PyMapping_Items(values);
		if (items){
			unsigned int size=PyList_Size(items);
			for (unsigned int i=0; i<size; i++) {
				ReferData name, value;
				get_key_value(items, i, &name, &value);

				err=set_form_variable(name.P, value.P);
			}
			Py_DECREF(items);
		}
	}
	if (cookies){
		PyObject*items=PyMapping_Items(cookies);
		if (items){
			unsigned int size=PyList_Size(items);
			for (unsigned int i=0; i<size; i++) {
				ReferData name, value;
				get_key_value(items, i, &name, &value);
				err=set_cookie(name.P, value.P);
			}
			Py_DECREF(items);
		}
	}
	if (headers){
		PyObject*items=PyMapping_Items(headers);
		if (items){
			unsigned int size=PyList_Size(items);
			for (unsigned int i=0; i<size; i++) {
				ReferData name, value;
				get_key_value(items, i, &name, &value);
				err=set_header_variable(name.P, value.P);
			}
			Py_DECREF(items);
		}
	}
	return err;
}
PyObject* PyBrowser::connectBrowser(PyBrowser* self, PyObject *args, PyObject *kwds){
    char *host=0;
    long port=0;
	int to_release=0; //default 0: do NOT release; -1: don't care
    static char *kwlist[] = {"host", "port", "release", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "si|i", kwlist, &host, &port, &to_release))
        return 0; 

	if (!self) return 0; 

	int err=-1;
	if (self->browser_type==PB_IRB_BROWSER){
		err=((IrbNavigationBrowser*) ((PathNavigationBrowser*) self->browser))->connectBrowser(host, port, to_release, 0);
	}
	
	PyObject *result = Py_BuildValue("i", (char*) err);
	return result; 	
}
PyObject* PyBrowser::setTimeout(PyBrowser* self, PyObject *args, PyObject *kwds){
    long connect_time=0;
    long transfer_time=0;
    static char *kwlist[] = {"connect", "transfer", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "ii", kwlist, &connect_time, &transfer_time))
        return 0; 

	if (!self) return 0; 

	if (self->browser_type==PB_UNKNOWN) self->new_browser(PB_SOCKBROWSER); 

	int err=0;
	if (self->browser_type==PB_SOCKBROWSER){
		((HtBrowser*) self->browser)->Html.TimeoutConnect=connect_time; 
		((HtBrowser*) self->browser)->Html.TimeoutTransfer=transfer_time; 
	}else if (self->browser_type==PB_IRB_BROWSER){
		
	}
	
	PyObject *result = Py_BuildValue("i", (char*) err);
	return result; 	
}

PyObject* PyBrowser::setPage(PyBrowser* self, PyObject *args, PyObject *kwds){
    char *page=0;
    char *url=0;
    static char *kwlist[] = {"page", "url", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s|s", kwlist, &page, &url))
        return 0; 

	if (!self) return 0; 

	int err=self->set_page(page, url);
	
	PyObject *result = Py_BuildValue("i", (char*) err);
	return result; 

}
PyObject* PyBrowser::getPage(PyBrowser* self){
	ReferData page, url;
	self->get_page(&page, &url);

	/*
	if (page.L>=2 && page.P[0]==-1 && page.P[1]==-2){ //special treatment for google trend data
		long j=0; 
		for (long i=2; i<page.L; i++){
			if (page.P[i]){
				page.P[j++]=page.P[i]; 
			}
		}
		page.L=j; 
	}*/

	PyObject *result = Py_BuildValue("ss", page.P, url.P);
	return result; 
	
}
PyObject* PyBrowser::getUpdatedPage(PyBrowser* self){
	ReferData page, url;
	self->get_updated_page(&page, &url);

	/*
	if (page.L>=2 && page.P[0]==-1 && page.P[1]==-2){ //special treatment for google trend data
		long j=0; 
		for (long i=2; i<page.L; i++){
			if (page.P[i]){
				page.P[j++]=page.P[i]; 
			}
		}
		page.L=j; 
	}*/

	PyObject *result = Py_BuildValue("ss", page.P, url.P);
	return result; 
}

PyObject * PyBrowser::getattr(PyBrowser *self, char *name)
{
	if (!name) return 0; 

	if (!stricmp(name, "Page")){
		return getPage(self);
	}else if (!stricmp(name, "UpdatedPage")){
		return getUpdatedPage(self);
	}
	return 0;
}

int PyBrowser::setattr(PyBrowser *self, char *name, PyObject *v)
{
	if (!stricmp(name, "Page")){
		char* page=0, *url=0;
		if (!PyArg_ParseTuple(v, "(ss)", &page, &url)) return 0;

		int err=self->set_page(page, url); 

		return err; 
	}
    return 0;
}
PyObject* PyBrowser::runFunction(PyBrowser *self, PyObject *args, PyObject *kwds){
	char* query=0;
    static char *kwlist[] = {"query", NULL};

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s", kwlist, &query))
        return 0; 

	if (!self) return 0; 

	ReferData data; 
	if (self->browser_type==PB_SOCKBROWSER){
	}else if (self->browser_type==PB_IRB_BROWSER){
		((IrbNavigationBrowser*) self->browser)->sendBrowserCommand(query, &data); 
		if (data.L>=2){
			data.L-=2;
			data.P[data.L]=0;
		}
	}
	PyObject *result = Py_BuildValue("s", data.P);
	return result; 	
}





void* PyBrowser::new_browser(int type){
	if (type != browser_type)
		delete_browser();

	browser_type=type; 
	if (browser_type==PB_SOCKBROWSER){ 
		browser=(void*) new HtBrowser; 
	}else if (browser_type==PB_IRB_BROWSER){
		browser=(PathNavigationBrowser*) new IrbNavigationBrowser; 
	}

	return browser;
}

void PyBrowser::delete_browser(){
	if (browser){
		if (browser_type==PB_UNKNOWN || browser_type==PB_SOCKBROWSER) 
			delete (HtBrowser*) browser;
		else if (browser_type==PB_IRB_BROWSER) 
			delete (IrbNavigationBrowser*) ((PathNavigationBrowser*) browser);
	}
	browser=0; 

	if (url_helper){
		delete url_helper; 
	}
	url_helper=0; 

	browser_type=PB_UNKNOWN;
}

int PyBrowser::set_url(const char* url){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		err=((HtBrowser*) browser)->setUrl(url); 
	}else if (browser_type==PB_IRB_BROWSER){
		navigation_url=url;
		navigation_type=0;
	}
	return err;
}
int PyBrowser::navigate(){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		err=((HtBrowser*) browser)->navigate();
	}else if (browser_type==PB_IRB_BROWSER){
		if (navigation_type==0){
			((PathNavigationBrowser*) browser)->navigate(navigation_url.P, 40,-1); 
		}else{
			((PathNavigationBrowser*) browser)->submitForm(form_htql.P, input_htql.P, 40, -1);
		}
	}
	return err;
}

int PyBrowser::set_form_variable(const char* name, const char* value){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		err=((HtBrowser*) browser)->setVariable(name, value); 
	}else if (browser_type==PB_IRB_BROWSER){
		err=((PathNavigationBrowser*) browser)->setFormValue(name, value);
	}
	return err;
}
int PyBrowser::set_header_variable(const char* name, const char* value){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		err=((HtBrowser*) browser)->setHeader(name, value); 
	}else if (browser_type==PB_IRB_BROWSER){
		//err=((PathNavigationBrowser*) browser)->setHeader(name, value);
	}
	return err;
}

int PyBrowser::set_cookie(const char* name, const char* value){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		err=((HtBrowser*) browser)->setCookie(name, value); 
	}else if (browser_type==PB_IRB_BROWSER){
		err=((PathNavigationBrowser*) browser)->setCookie(name, value);
	}
	return err;
}
int PyBrowser::use_form_query(const char* form_htql1){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		err=((HtBrowser*) browser)->useForm(form_htql1); 
	}else if (browser_type==PB_IRB_BROWSER){
		form_htql=form_htql1;
		input_htql.reset();
		err=((PathNavigationBrowser*) browser)->useForm(form_htql.P);
		navigation_type=1;
	}
	return err;
}
int PyBrowser::use_form_index(int form_index1){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		err=((HtBrowser*) browser)->useForm(form_index1); 
	}else if (browser_type==PB_IRB_BROWSER){
		char buf[256];
		sprintf(buf, "<form>%d", form_index1); 
		form_htql=buf;
		err=((PathNavigationBrowser*) browser)->useForm(form_htql.P);
		navigation_type=1;
	}
	return err;
}
int PyBrowser::click_item(const char* item_htql, int wait){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		err=((HtBrowser*) browser)->clickItem(item_htql); 
	}else if (browser_type==PB_IRB_BROWSER){
		err=((PathNavigationBrowser*) browser)->clickPageItem(item_htql, wait, -1);
	}
	return err;
}


int PyBrowser::get_page(ReferData* page, ReferData* url){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		page->Set( ((HtBrowser*) browser)->getContentData(), ((HtBrowser*) browser)->getContentLength(), false); 
		*url=((HtBrowser*) browser)->Url;
	}else if (browser_type==PB_IRB_BROWSER){
		err=((PathNavigationBrowser*) browser)->getPageSource(page);
		if (err>=0) err=((PathNavigationBrowser*) browser)->getPageUrl(url);
	}
	return err;
}
int PyBrowser::get_updated_page(ReferData* page, ReferData* url){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		page->Set( ((HtBrowser*) browser)->getContentData(), ((HtBrowser*) browser)->getContentLength(), false); 
		*url=((HtBrowser*) browser)->Url;
	}else if (browser_type==PB_IRB_BROWSER){
		err=((PathNavigationBrowser*) browser)->getUpdatedSource(page);
		if (err>=0) err=((PathNavigationBrowser*) browser)->getUpdatedUrl(url);
	}
	return err;
}

int PyBrowser::set_page(const char*page, const char* url){
	int err=0;
	if (browser_type==PB_SOCKBROWSER){
		if (page){
			((HtBrowser*) browser)->Html.Buffer.SetValue(page, strlen(page));
			((HtBrowser*) browser)->Html.reset(); 
		}
		if (url) 
			((HtBrowser*) browser)->Url.Set((char*) url, strlen(url), true);  
	}else if (browser_type==PB_IRB_BROWSER){
		ReferData page1, url1;
		page1.Set((char*) page, page?strlen(page):0, false); 
		url1.Set((char*) url, url?strlen(url):0, false); 
		err=((PathNavigationBrowser*) browser)->setBufferedPage(&url1, &page1);
	}
	return err;
}


