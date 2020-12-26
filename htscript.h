#ifndef CLY_HT_SCRIPT_H_20030110
#define CLY_HT_SCRIPT_H_20030110

#include "qlsyntax.h"
#include "referdata.h"
#include "referlink.h"

#define HTS_ERR_SYNTAX_ERROR -100
#define HTS_ERR_NOINTERFACE -101
#define HTS_ERR_NOTVARIABLE	-102
#define HTS_ERR_NOTCOMMAND	-103
#define HTS_ERR_EXISTING_INTERFACE	-104
#define HTS_ERR_VARIABLE_NOTFOUND	-105
#define HTS_ERR_SCRIPTFILE_NOTFOUND -106

class HtScriptSyntax: public QLSyntax{
public:
	enum {synHT_SCRIPT=100, synHT_END};
	virtual void findNext();
	virtual int KeyWord();
	virtual int isComment(char*, long* len);
};

class HtScriptVariableSyntax: public QLSyntax{
public:
	virtual void findNext();

//	HtScriptVariableSyntax();
//	~HtScriptVariableSyntax();
};

class HtScript;
class HtScriptEnv{
public:
	HtScript* Script;
	ReferLink* CurrentLine;
	ReferLink* CurrentDefVar;
	void* Pointer;

	HtScriptEnv& operator = (HtScriptEnv&);
	HtScriptEnv();
};

typedef enum {htSC_UNKNOW, htSC_LONG, htSC_DOUBLE, htSC_STRING, htSC_POINTER, htSC_VARIABLE} htSC_TYPE;
#if !defined(SCRIPT_LOG_FILE)
#define SCRIPT_LOG_FILE "script.log"
#endif

class HtScriptInterface {
public:
	ReferData InterfaceName;
	int IsAsynchronous;
	void* Pointer;
	virtual int isCommand(char* command);
	virtual void* execCommand(HtScript* callfrom, void* var, char* command, void* arg1=0, void* arg2=0, void* arg3=0, void* arg4=0);
	virtual void* createVariable();
	virtual void deleteVariable(void* var);

	int addLog(
		char* log_file, char* source_file, int source_line, 
		char* command,
		void* ret=0,  htSC_TYPE ret_type=htSC_UNKNOW,
		void* arg1=0, htSC_TYPE type1=htSC_UNKNOW, 
		void* arg2=0, htSC_TYPE type2=htSC_UNKNOW, 
		void* arg3=0, htSC_TYPE type3=htSC_UNKNOW, 
		void* arg4=0, htSC_TYPE type4=htSC_UNKNOW);

	HtScriptInterface();
	virtual ~HtScriptInterface();
	void reset();

protected:
	int strArg(void* arg, htSC_TYPE type, ReferData* buf);
};

class HtScript{
public:
	ReferLinkHeap Variables; //Name: name; Data: htSC_TYPE; Value.P: variable; Value.L: interface-link
	ReferLinkHeap ScriptInterfaces;	//Name: name; Data: HtScriptInterface;
	ReferLink* ScriptContext; //Value.P: variable
	ReferLink* ScriptLines;	//Name: command; Data: variable-link; Value.P: args[4]; 
						//args[i]->Value.P: argument pointer;
	ReferLink* LastScriptLine;

	int IsAsynchronousCommand;
	int IsAsynchronous;
	HtScriptEnv CurrentEnv;

	int setVariable(ReferData* varname, ReferData* varvalue);
	ReferLink* getDefVariable();
	ReferLink* getVariable(ReferData* varname);

	int registerInterface(HtScriptInterface* interf, ReferData* name);
	int executeScript(char* script, long len=0);
	int executeScriptFile(char* filename);
	int setOnCompleteCall(void (*fun)(HtScript*, void* ), void* para);

	HtScript();
	~HtScript();
	void reset();

	int executeSubScript(ReferLink* lines, ReferLink* def_var=0, ReferLink** stop_line=0);
protected:
	int parseScript(const char* script, long len=0, ReferLink** first_line=0);

	void* executeCommand(HtScriptInterface* interf, void* var, char* command, ReferLink** args);
	ReferLink* newScriptLine();
	ReferLink* newVariableLink(ReferData* varname);
	int computeArgValue(ReferLink* arg);
	ReferLink* findVariable(ReferData* varname);
	ReferLink* addInterfaceVariable(ReferData* varname);
	int computeVarValue(ReferData* varname, ReferLink* ret_res);

	ReferLink* findCommandContext(ReferData* command);//not implemented yet
	ReferLink* addCommandContext(ReferLink* var);
	int removeCommandContext(ReferLink* var);

	void* OnCompleteCallPara;
	void (*OnCompleteCall)(HtScript*, void* p);
};


class HtBrowserScriptInterface: public HtScriptInterface{
public:
	virtual int isCommand(char* command);
	virtual void* execCommand(HtScript* callfrom, void* var, char* command, void* arg1=0, void* arg2=0, void* arg3=0, void* arg4=0);
	virtual void* createVariable();
	virtual void deleteVariable(void* var);

	HtBrowserScriptInterface();
	virtual ~HtBrowserScriptInterface();
};

class HtHtqlScriptInterface: public HtScriptInterface{
public:
	virtual int isCommand(char* command);
	virtual void* execCommand(HtScript* callfrom, void* var, char* command, void* arg1=0, void* arg2=0, void* arg3=0, void* arg4=0);
	virtual void* createVariable();
	virtual void deleteVariable(void* var);

	HtHtqlScriptInterface();
	virtual ~HtHtqlScriptInterface();
};

class HtStringScriptInterface: public HtScriptInterface{
public:
	virtual int isCommand(char* command);
	virtual void* execCommand(HtScript* callfrom, void* var, char* command, void* arg1=0, void* arg2=0, void* arg3=0, void* arg4=0);
	virtual void* createVariable();
	virtual void deleteVariable(void* var);

	HtStringScriptInterface();
	virtual ~HtStringScriptInterface();
};


class HtBrowserScript: public HtScript {
public:
	static HtBrowserScriptInterface BrowserInteface;
	static HtHtqlScriptInterface HtqlScriptInterface;
	static HtStringScriptInterface StringScriptInterface;

	HtBrowserScript();
	~HtBrowserScript();
	void reset();
};

#endif
