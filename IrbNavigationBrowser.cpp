
#include "IrbNavigationBrowser.h"
#include "htmlbuf.h"
#include "stroper.h"
#include "dirfunc.h"

#ifdef _WINDOWS
	#include <shellapi.h>
#endif
#ifdef _WINDOWS
	#ifndef VC6
		#include "atlstr.h"
	#else
		#include "atlbase.h"
		#include "comutil.h"
	#endif
#endif



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_THIS_FILE
#endif


IrbNavigationBrowser::IrbNavigationBrowser(){
	Name="IrbNavigationBrowser";
	Connector=0;
	ToExitIRobot=false;

	ToSaveLastVisitState=0;
	LastVisitTime=0;
	DirFunc::getTempPath(&TempPath);

	Port=nextAvailablePort(); 
}

IrbNavigationBrowser::~IrbNavigationBrowser(){
	reset();
}

void IrbNavigationBrowser::reset(){
	disconnectBrowser();
	ToExitIRobot=false;

	ToSaveLastVisitState=0;
	LastVisitTime=0;
	LastVisitPage.reset();

	PathNavigationBrowser::reset();
}
long IrbNavigationBrowser::nextAvailablePort(){
	ReferData port_file=TempPath; 
	port_file+="IrbNavigationBrowser.port";

	ReferData port_id; 
	port_id.readFile(port_file.P); 
	long port=port_id.getLong();
	if (!port || port>=99999) port=2011; 

	port_id.setLong(port+1); 
	port_id.saveFile(port_file.P); 

	return port;
}

int IrbNavigationBrowser::runCommand(const char* appname, const char* param, const char* rundir, long showopt, ReferData* errmsg){
#ifdef _WINDOWS
	USES_CONVERSION;
	//change to CreateProcess() later
	int err=(int)ShellExecute(0, A2T((char*) "open"), A2T((char*) appname), A2T((char*) param), A2T((char*) rundir), showopt); 

	if (err<=32 && errmsg){
		char buf[64]; 
		sprintf(buf, "%d", err); 
		*errmsg+="<p>ERROR: <a id='ERROR' error='"; *errmsg+=buf; *errmsg+="'>";

		switch (err){
		case 0: *errmsg+="The operating system is out of memory or resources!"; break;
		case ERROR_FILE_NOT_FOUND: *errmsg+="The specified file was not found!"; break;
		case ERROR_PATH_NOT_FOUND : *errmsg+="The specified path was not found!"; break;
		case ERROR_BAD_FORMAT: *errmsg+="The .exe file is invalid (non-Win32? .exe or error in .exe image).!"; break;
		case SE_ERR_ACCESSDENIED: *errmsg+="The operating system denied access to the specified file.!"; break;
		case SE_ERR_ASSOCINCOMPLETE : *errmsg+="The file name association is incomplete or invalid!"; break;
		case SE_ERR_DDEBUSY: *errmsg+="The DDE transaction could not be completed because other DDE transactions were being processed!"; break;
		case SE_ERR_DDEFAIL: *errmsg+="The DDE transaction failed!"; break;
		case SE_ERR_DDETIMEOUT: *errmsg+="The DDE transaction could not be completed because the request timed out!"; break;
		case SE_ERR_DLLNOTFOUND: *errmsg+="The specified dynamic-link library was not found!"; break;
		case SE_ERR_NOASSOC: *errmsg+="There is no application associated with the given file name extension!"; break;
		case SE_ERR_OOM: *errmsg+="There was not enough memory to complete the operation!"; break;
		case SE_ERR_SHARE: *errmsg+="A sharing violation occurred!"; break;
		default: *errmsg+="Unknown error!"; break;
		}
		*errmsg+="</a></p>\n";
		TRACE("%s\r\n", errmsg->P);
	}
#else
	int err=0;
#endif
	return err;
}
int IrbNavigationBrowser::getIRobotFullName(ReferData* fullname, ReferData* fullpath){
	int err=0;
#ifdef _WINDOWS
	USES_CONVERSION;
	HKEY program;
	unsigned char name[256]="";
	unsigned char value[256]="";
	unsigned long len=255;
	DWORD type=REG_SZ;
	int success=true;

	err=RegOpenKey(HKEY_CURRENT_USER, A2T("Software\\IRobotSoft\\irobot\\Module"), &program);
	len=255;
	if (!err) err=RegQueryValueEx(program, A2T("Executable"), 0, &type, value, &len);
	if (!err) *fullname=(char*) value;
	else *fullname="irobot.exe";
#endif
	return err;
}

int IrbNavigationBrowser::connectBrowser(const char* host, long port, int to_release, int to_start){
	disconnectBrowser(); 

	if (host) ServerAddr=host; 
	if (port) Port=port;
	if (to_release>=0) ToExitIRobot=to_release;

	if (!Connector){
		Connector=new HtmlBuffer;
	}
	int err=0;
   	if (!ServerAddr.L || (!ServerAddr.Cmp("localhost", 9, false) && to_start)) { //( !ServerAddr.Cmp("localhost", 9, false) && to_release!=0 ) ) {
		ServerAddr="localhost";

		//run irb browser
		ReferData exename, exepath; 
		err=getIRobotFullName(&exename, &exepath);

		char param[256];
		sprintf(param, "/listen:%d /No", Port); 
		ReferData msg;
#ifdef _WINDOWS
		err=runCommand(exename.P, param, exepath.P, SW_SHOWNORMAL, &msg);
#else
		err=runCommand(exename.P, param, exepath.P, 0, &msg);
#endif
		if (to_release<0) ToExitIRobot=true; //to_release<0: don't care
	}
	//sleep(4);
	Connector->TimeoutConnect=120;
	Connector->TimeoutTransfer=120;
	err=Connector->connectHost(ServerAddr.P, Port, tPort::ssNone); 
	if (err<0) {
		delete Connector; 
		Connector=0;
	}

	Connector->TimeoutConnect=1000;
	Connector->TimeoutTransfer=1000;
	ReferData result; 
	err=sendBrowserCommand("hello()", &result); 
	return err;
}
int IrbNavigationBrowser::disconnectBrowser(){
	ReferData result;
	if (Connector){
		Connector->TimeoutConnect=10;
		Connector->TimeoutTransfer=10;
	}
	if (Connector && ToExitIRobot){
		int err=sendBrowserCommand("exit", &result);
	}else if (Connector) {
		int err=sendBrowserCommand("bye", &result);
	}
	if (Connector) delete Connector; 
	Connector=0;
	ToExitIRobot=false;
	
	return 0;
}
int IrbNavigationBrowser::sendBrowserCommand(const char* command, ReferData* result){
	if (!Connector) connectBrowser(); 
	if (!Connector) return -1;

	ReferData msg; msg=command; msg+="\r\n"; 
	int err=Connector->Server->put(msg.P);
	if (err>=0) err=Connector->prefetch(0); 
	if (err>=0) err=Connector->fetchHtmlContent(false); 

	if (result){
		if (err>=0) *result = Connector->getContentData();
		else result->reset(); 
	}

#ifdef DEBUG_THIS_FILE
	ReferData debug_str; 
	if (result->L<100) debug_str=*result;
	else debug_str.Set(result->P, 100, true); 
	TRACE("%s -> %d: %s\r\n", command, err, debug_str.P);
#endif
	return err;
}

int IrbNavigationBrowser::getPageSource(ReferData* source){
	return sendBrowserCommand("getPage()", source); 
}
int IrbNavigationBrowser::getPageUrl(ReferData* url){
	return sendBrowserCommand("getUrl()", url); 
}
int IrbNavigationBrowser::getUpdatedSource(ReferData* source){
	return sendBrowserCommand("getUpdatedPage()", source); 
}
int IrbNavigationBrowser::getUpdatedUrl(ReferData* url){
	return sendBrowserCommand("getUpdatedUrl()", url); 
}


int IrbNavigationBrowser::navigate(const char* url, int max_wait_sec, int nav_speed){
	ReferData result, q;
	ReferData command; command="goURL('"; command+=tStrOp::quoteString(url, &q); command+="')"; 
	sendBrowserCommand(command.P, &result); 

	if (!result.Cmp("OK",2,false)) return 0; 
	return -1;
}
int IrbNavigationBrowser::clickPageItem(const char* item_htql, int to_wait_sec, int nav_speed, PathNavigationBrowser* open_to_browser){
	ReferData result, q;
	ReferData command; command="click('"; command+=tStrOp::quoteString(item_htql, &q); command+="')"; 
	sendBrowserCommand(command.P, &result); 

	if (!result.Cmp("OK",2,false)) return 0; 
	return -1;
}
int IrbNavigationBrowser::useForm(const char* form_htql){
	ReferData command, result, q; 
	command = "useForm('"; command+=tStrOp::quoteString(form_htql, &q); command+="')"; 
	sendBrowserCommand(command.P, &result); 
	return result.getLong();
}
int IrbNavigationBrowser::setFormValue(const char* name, const char* value){
	ReferData command, result, q; 
	command="setFormValue('"; 
	command+=tStrOp::quoteString(name, &q); command+="','";
	command+=tStrOp::quoteString(value, &q); command+="')"; 
	sendBrowserCommand(command.P, &result); 
	return result.getLong();
}
/*
int IrbNavigationBrowser::fillBrowserForm(const char* form_htql, ReferData* webpath, const char* group_name, PathNavigationTree* tree, int mismatch_level){
	HTQL ql;
	ql.setSourceData(webpath->P, webpath->L, false);
	ql.setGlobalVariable("g", group_name);

	ReferData group_action;
	//if (info) info->getVariable("LAWEB_FORM_ACTION", &group_action);
	//change to use tree->getVariable()
	if (tree) tree->getVariable("LAWEB_FORM_ACTION", &group_action, false);

	ReferData page, url;
	getUpdatedSource(&page); 
	getUpdatedUrl(&url);

	HtmlQL browserql; 
	browserql.setSourceData(page.P, page.L, false); 
	browserql.setQuery(form_htql); 
	browserql.dotQuery(":action &url");
	ReferData form_action = browserql.getValue(1); 

	if (mismatch_level==0 && PathNavigationBrowser::cmpFormAction(url.P, form_action.P, group_action.P)){
		return -1;
	}

	//Connector.setQuery(form_htql);
	//Connector.dotQuery(":action &url");
	//ReferData form_action=Connector.getValue(1);

	//if (mismatch_level==0 && PathNavigationBrowser::cmpFormAction(Connector.Url.P, form_action.P, group_action.P)){
	//	return -1;
	//}

	ReferData command, result, q; 
	command = "useForm('"; command+=tStrOp::quoteString(form_htql, &q); command+="')"; 
	sendBrowserCommand(command.P, &result); 
	//Connector.useForm(form_htql);
	ql.setQuery("<LaWebInputs>.<LaWebInput (Group=g and Name<>'LAWEB_FORM_ACTION' and Comment<>'TRUE')>{Name=:Name; Type=:Type; Value=:tx &html_decode;}");
	ReferData name, value, var_value;
	for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
		value = ql.getValue("Value");
		name = ql.getValue("Name");
		//if (info && info->getVariable(name.P, &var_value)){ //get variable to replace value
		//change to use tree->getVariable()
		if (tree && tree->getVariable(name.P, &var_value, false)){ //get variable to replace value
			value=var_value;
		}
		command="setFormValue('"; command+=tStrOp::quoteString(form_htql, &q); command+="','";
		command+=tStrOp::quoteString(name.P, &q); command+="','";
		command+=tStrOp::quoteString(value.P, &q); command+="')"; //need to quote text
		sendBrowserCommand(command.P, &result); 
		//Connector.setVariable(name.P, value.P);
	}
	return 0;
}
*/
int IrbNavigationBrowser::submitForm(const char* form_htql, const char*sbm_input_htql, int max_wait_sec, int nav_speed, PathNavigationBrowser* open_to_browser){
	ReferData command, result, q; 
	char buf[256]; 
	command="submitForm('"; command+=tStrOp::quoteString(form_htql, &q); command+="','";
	command+=tStrOp::quoteString(sbm_input_htql, &q); command+="',"; 
	sprintf(buf, "%d,%d,'", max_wait_sec, nav_speed); command+=buf; command+=(char*) open_to_browser; command+="')"; 
	sendBrowserCommand(command.P, &result); 

	return result.getLong(); 
}

int IrbNavigationBrowser::setCookie(const char* name, const char* value){
	ReferData command, result, q; 
	command="setCookie('"; command+=tStrOp::quoteString(name, &q); 
	command+="','"; command+=tStrOp::quoteString(value, &q); command+="')"; 
	sendBrowserCommand(command.P, &result); 
	return result.getLong(); 
}
int IrbNavigationBrowser::getCookie(const char* name, ReferData* value){
	ReferData command, q; 
	command="getCookie('"; command+=tStrOp::quoteString(name, &q); command+="')"; 
	return sendBrowserCommand(command.P, value); 
}
int IrbNavigationBrowser::getCookies(ReferLinkHeap* cookies){
	ReferData cookies_str; 
	sendBrowserCommand("getCookies()", &cookies_str); 
	return tStrOp::parseCookiesString(cookies_str.P, cookies);
	//return Connector.getCookie(0, 0, cookies);
}

int IrbNavigationBrowser::useFrame(int index1, const char* frames_htql){
	ReferData command, value, q; 
	char buf[256]; 
	command="useFrame("; sprintf(buf, "%d, '", index1); command+=buf; 
	command+=tStrOp::quoteString(frames_htql, &q); command+="')"; 
	sendBrowserCommand(command.P, &value); 
	return value.getLong();

/*	char buf[128];
	if (!frames_htql || !frames_htql[0]){
		if (index1>0){
			sprintf(buf, "<frame>%d: href", index1);
			Connector.setQuery(buf);
			if (Connector.isEOF()){
				sprintf(buf, "<iframe>%d: href", index1);
				Connector.setQuery(buf);
			}
			char* url=Connector.getValue(1);
			if (url && strlen(url)){
				Connector.setUrl(url);
				Connector.navigate();
			}
		}
	}else{
		HtmlQL ql; 
		ql.setSourceData(frames_htql, strlen(frames_htql), false);
		ql.setQuery("/'.'/");
		for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
			char* p=ql.getValue(1);
			if (strlen(p)){
				size_t len=TagOperation::getTagLength(p);
				useFrame(atoi(p+len), 0);
			}
		}
	}
	return 0;
	*/
}

int IrbNavigationBrowser::setBufferedPage(ReferData* url, ReferData* page, int copy, ReferLink* cookies){
	//to change later ...


/*	int err=PathNavigationBrowser::setBufferedPage(url, page, copy);
	if (err<0) return err;
	Connector.Html.Buffer.SetValue(BufferedPage->Source.P, BufferedPage->Source.L);
	Connector.Html.ContentLength = Connector.Html.Buffer.DataLen;
	Connector.Html.Url.SetValue(url->P, url->L);
	ReferLink* link;
	for (link=cookies; link; link=link->Next){
		Connector.setCookie(link->Name.P, link->Value.P);
	}*/
	return 0;
}


