#include "htscript.h"
#include "stroper.h"
#include "qhtql.h"
#include <stdio.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define SCRIPT_LOG_FILE_OUTPUT

//================================================

HtScriptInterface::HtScriptInterface(){
	Pointer=0;
	IsAsynchronous=0;
}

HtScriptInterface::~HtScriptInterface(){
	reset();
}

void HtScriptInterface::reset(){
	//InterfaceName.reset();
	Pointer=0;
	IsAsynchronous=0;
}

void* HtScriptInterface::execCommand(HtScript* callfrom, void* var, char* command, void* arg1, void* arg2, void* arg3, void* arg4){
	return 0;
}

int HtScriptInterface::strArg(void* arg, htSC_TYPE type, ReferData* buf){
	char tmp[40];
		if (type == htSC_STRING){
			buf->Cat((char*) arg, strlen((char*) arg) );
		}else if (type == htSC_DOUBLE){
			sprintf(tmp, "%lf",  ((long) arg)/10000.00);
			buf->Cat(tmp, strlen(tmp));
		}else if (type == htSC_LONG){
			sprintf(tmp, "%ld",  (long) arg);
			buf->Cat(tmp, strlen(tmp));
		}else if (type == htSC_POINTER){
			sprintf(tmp, "->%ld",  (long) arg );
			buf->Cat(tmp, strlen(tmp));
		}else if (type == htSC_VARIABLE){
			sprintf(tmp, "@%ld",  (long) arg );
			buf->Cat(tmp, strlen(tmp));
		}else {
			sprintf(tmp, "??%ld",  (long) arg );
			buf->Cat(tmp, strlen(tmp));
		}
		return 0;
}

int HtScriptInterface::addLog(
		char* log_file, char* source_file, int source_line, 
		char* command,
		void* ret,  htSC_TYPE ret_type,
		void* arg1, htSC_TYPE type1, 
		void* arg2, htSC_TYPE type2, 
		void* arg3, htSC_TYPE type3, 
		void* arg4, htSC_TYPE type4){

#ifdef SCRIPT_LOG_FILE_OUTPUT
	char tmp[50]="";
	tStrOp::DateToChar(time(NULL),"YYYY/MM/DD HH:MI:SS Dy",tmp);

	ReferData str;
	str.Set(tmp, strlen(tmp), true);
	
	void* args[]={ret, arg1, arg2, arg3, arg4, 0};
	htSC_TYPE types[] = {ret_type, type1, type2, type3, type4, htSC_UNKNOW};

	str.Cat(": -- ", strlen(": -- ") );
	if (command) str.Cat(command, strlen(command));
	str.Cat("(): -- ", strlen("(): -- ") );
	if (source_file) str.Cat(source_file, strlen(source_file));
	str.Cat(", ", 2);
	sprintf(tmp, "%d", source_line);
	str.Cat(tmp, strlen(tmp));
	for (int i=1; args[i]; i++){
		sprintf(tmp, "\n\t\t%%%i : ", i);
		str.Cat(tmp, strlen(tmp));
		strArg(args[i], types[i], &str);
	}
	if (ret){
		sprintf(tmp, "\n\t\tReturn: ");
		str.Cat(tmp, strlen(tmp));
		strArg(ret, ret_type, &str);
	}
	
	FILE * flog;
	if ((flog=fopen(log_file,"a+"))==NULL) return -1;
	fprintf(flog,"%s", str.P);
	fprintf(flog,"\n");
	fclose(flog);
#endif
	return 0;
}

int HtScriptInterface::isCommand(char* command){
	return 0;
}

void* HtScriptInterface::createVariable(){
	return 0;
}	
void HtScriptInterface::deleteVariable(void* var){
	return ;
}


//================================================

void HtScriptSyntax::findNext(){
	if (Next>=Data.L){
		NextType = synQL_END;
		NextLen = 0;

	}else if (Sentence[Next] == '='){
		NextType = QLSyntax::synQL_EQ;
	}else if (Sentence[Next] == ','){
		NextType = synQL_COMMA;
	}else if (Sentence[Next] == ';'){
		NextType = synQL_SEMICOLON;
	}else if (Sentence[Next] == '('){
		NextType = synQL_LBRACE;
	}else if (Sentence[Next] == ')'){
		NextType = synQL_RBRACE;
	}else if (Sentence[Next] == '"'){
		int isSpecial=false;
		while (Next+NextLen < Data.L && (Sentence[Next+NextLen] != Sentence[Next] || isSpecial)) {
			if (Sentence[Next+NextLen] == '\\') isSpecial = !isSpecial;
			else isSpecial=false;
			NextLen++;
		}
		if (Next+NextLen < Data.L) 
			NextLen++;
		NextType = synQL_STRING;
	}else if (Sentence[Next] == '\''){
		//quote multiple lines; until a '\'' at the end of a line; 
		while (1){
			while (Next+NextLen<Data.L && Sentence[Next+NextLen]!='\r' && Sentence[Next+NextLen]!='\n')
				NextLen++;
			while (NextLen>0 && isSpace(Sentence[Next+NextLen]) || Sentence[Next+NextLen]=='\0') 
				NextLen--;
			if (Sentence[Next+NextLen]==';') 
				NextLen--;
			if (Sentence[Next+NextLen]=='\''){
				if (Next+NextLen < Data.L) 
					NextLen++;
				break;
			}else{
				NextLen++;
				if (Sentence[Next+NextLen]==';')
					NextLen++;
				while (Next+NextLen<Data.L && isSpace(Sentence[Next+NextLen]) )
					NextLen++;
				if (Next+NextLen>=Data.L || Sentence[Next+NextLen]=='\0')
					break;
			}
		}
		NextType = synQL_STRING;
	}else if (isAlpha(Sentence[Next]) ){
		while (Next+NextLen < Data.L && (isAlpha(Sentence[Next+NextLen])||isDigit(Sentence[Next+NextLen])|| Sentence[Next+NextLen]==':' || Sentence[Next+NextLen]=='.')){
			NextLen++;
		}
		NextType=synQL_WORD;

	}else if (isDigit(Sentence[Next]) ){
		while (Next+NextLen < Data.L && isDigit(Sentence[Next+NextLen])){
			NextLen++;
		}
		NextType=synQL_NUMBER;

	}else{
		NextType=synQL_UNKNOW;
		NextLen = 0;
	}
}

int HtScriptSyntax::KeyWord(){
	if (NextType!=synQL_WORD) return NextType;
	if ((NextLen==6)&&!tStrOp::strNcmp(Sentence+Next,"SCRIPT",NextLen, false)) return synHT_SCRIPT;
	if ((NextLen==3)&&!tStrOp::strNcmp(Sentence+Next,"END",NextLen, false)) return synHT_END;
	return synQL_WORD;
}

int HtScriptSyntax::isComment(char* p, long* len){
	if (p[0]=='/' && p[1]=='/'){
		*len=2;
		while (p[*len] && p[*len]!='\n' && p[*len]!='\r') (*len)++;
		if (p[*len]) (*len)++;
		return true;
	}else if (p[0]=='/' && p[1]=='*'){
		char* p1 = strstr(p, "*/");
		if (p1) {
			*len = p1-p+2;
			return true;
		}
	}
	return false;
}

void HtScriptVariableSyntax::findNext(){
	if (Next>=Data.L){
		NextType = synQL_END;
		NextLen = 0;

	}else if (Sentence[Next] == '.' || Sentence[Next] == ':'){
		NextType = QLSyntax::synQL_DOT;
	}else{
		while (Next+NextLen < Data.L && (isAlpha(Sentence[Next+NextLen])||isDigit(Sentence[Next+NextLen]) )){
			NextLen++;
		}
		NextType=synQL_WORD;
	}
}


HtScriptEnv::HtScriptEnv(){
	Script = 0;
	CurrentLine = 0;
	CurrentDefVar = 0;
	Pointer =0;
}

HtScriptEnv& HtScriptEnv::operator = (HtScriptEnv& source){
	Script = source.Script;
	CurrentLine = source.CurrentLine;
	CurrentDefVar = source.CurrentDefVar;
	Pointer = source.Pointer;
	return *this;
}

//==============================================================

HtScript::HtScript(){
	Variables.setDuplication(false);
	Variables.setSortOrder(SORT_ORDER_KEY_STR_INC);
	ScriptInterfaces.setDuplication(false);
	ScriptInterfaces.setSortOrder(SORT_ORDER_KEY_STR_INC);
	ScriptContext=0;
	ScriptLines=0;
	LastScriptLine=0;
	OnCompleteCallPara=0;
	OnCompleteCall=0;
}
HtScript::~HtScript(){
	reset();
}

void HtScript::reset(){
	HtScriptInterface* interf=0;
	ReferLink* link = (ReferLink*) Variables.moveFirst();
	while (link){
		if (link->Data == htSC_VARIABLE){
			interf =(HtScriptInterface*) link->Value.L;
			if (interf) interf->deleteVariable(link->Value.P);
		}

		link = (ReferLink*) Variables.moveNext();
	}
	Variables.reset();
	Variables.setDuplication(false);
	Variables.setSortOrder(SORT_ORDER_KEY_STR_INC);

/*	link = (ReferLink*) ScriptInterfaces.moveFirst();
	while (link){
		if (link->Data){
			interf =(HtScriptInterface*) link->Data;
			delete interf;
			link->Data=0;
		}

		link = (ReferLink*) Variables.moveNext();
	}*/
	ScriptInterfaces.reset();
	ScriptInterfaces.setDuplication(false);
	ScriptInterfaces.setSortOrder(SORT_ORDER_KEY_STR_INC);

	for (link = ScriptLines; link; link=link->Next){
		if (link->Value.P){
			ReferLink** args= (ReferLink**) link->Value.P;
			for (int i=0; i<4; i++){
				if (args[i]) delete args[i];
				args[i]=0;
			}
		}
	}

	if (ScriptLines) {
		delete ScriptLines;
		ScriptLines=0;
	}

	LastScriptLine=0;

	if (ScriptContext){
		delete ScriptContext;
		ScriptContext=0;
	}
	OnCompleteCallPara=0;
	OnCompleteCall=0;
}

int HtScript::registerInterface(HtScriptInterface* interf, ReferData* name){
	ReferLink* link = ScriptInterfaces.findName(name);
	if (link) {
		link->Data = (long) interf;
		return 1;
	}else{
		ReferLink* link = ScriptInterfaces.add(name, 0, (long) interf);
		return 0;
	}
}

int HtScript::parseScript(const char* script, long len, ReferLink** first_line){
	HtScriptSyntax syntax;
	syntax.setSentence(script, len?(&len):0, false);

	ReferData varname;
	ReferData command;
	ReferLink* var;
	ReferLink* line;
	ReferLink* firstline = 0;
	while (syntax.Type!=QLSyntax::synQL_END && syntax.Type!=QLSyntax::synQL_UNKNOW) {
		if (syntax.Type != QLSyntax::synQL_WORD) return HTS_ERR_SYNTAX_ERROR;
		varname.reset();
		var=0;
		if (syntax.NextType == QLSyntax::synQL_EQ){
			varname.Set(syntax.Sentence+syntax.Start, syntax.StartLen, true);
			syntax.match();	//word

			var = Variables.findName(&varname);
			if (!var){
				var=Variables.add(&varname, 0, 0);
			}
			syntax.match();//=
		}
		if (syntax.Type==QLSyntax::synQL_END || syntax.Type==QLSyntax::synQL_SEMICOLON) return HTS_ERR_SYNTAX_ERROR;
		command.Set(syntax.Sentence+syntax.Start, syntax.StartLen, true);
		syntax.match();

		line = newScriptLine();
		if (!firstline){
			firstline = line;
			if (first_line) *first_line = firstline;
		}
		line->Name.Set(command.P, command.L, true);
		if (var) {
			line->Data = (long) var;
		}
		
		if (syntax.Type == QLSyntax::synQL_LBRACE)
			syntax.match();
		ReferLink** args=0;
		int cur_arg=0;
		while (syntax.Type!=QLSyntax::synQL_UNKNOW && syntax.Type!=QLSyntax::synQL_END && syntax.Type!=QLSyntax::synQL_SEMICOLON && syntax.Type != QLSyntax::synQL_RBRACE){
			if (!cur_arg) {
				line->Value.Malloc(sizeof(ReferLink**)*4);
				args=(ReferLink**) line->Value.P;
				for (int i=0; i<4; i++) args[i]=0;
			}

			if (syntax.Type != QLSyntax::synQL_COMMA) {
				if (cur_arg<4){
					args[cur_arg]=new ReferLink;
					/*
					if (isdigit(syntax.Sentence[syntax.Start])){
						long i=syntax.Start;
						while (isdigit(syntax.Sentence[i])) i++;
						if (syntax.Sentence[i]=='.'){
							args[cur_arg]->Name.Malloc(sizeof(double));
							memset(args[cur_arg]->Name.P, 0, args[cur_arg]->Name.L);
							sscanf(syntax.Sentence+syntax.Start, "%lf", (double*) args[cur_arg]->Name.P);
						}else{
							args[cur_arg]->Name.Malloc(sizeof(long));
							memset(args[cur_arg]->Name.P, 0, args[cur_arg]->Name.L);
							sscanf(syntax.Sentence+syntax.Start, "%ld", (long*) args[cur_arg]->Name.P);
						}
					}else if (syntax.Sentence[syntax.Start]=='\''||syntax.Sentence[syntax.Start]=='"'){
						args[cur_arg]->Name.Set(syntax.Sentence+syntax.Start+1, syntax.StartLen-2, true);
					}else{
						args[cur_arg]->Name.Set(syntax.Sentence+syntax.Start, syntax.StartLen, true);
					}*/
						args[cur_arg]->Name.Set(syntax.Sentence+syntax.Start, syntax.StartLen, true);
				}
				syntax.match();
			}

			if (syntax.Type == QLSyntax::synQL_COMMA) 
				syntax.match();

			cur_arg++;
		}
		if (syntax.Type == QLSyntax::synQL_RBRACE)
			syntax.match();

		if (syntax.Type == QLSyntax::synQL_SEMICOLON)
			syntax.match();
	}
	return 0;
}

int HtScript::executeScript(char* script, long len){
	int i;
	ReferLink* first_line=0;
	if ((i=parseScript(script, len, &first_line))<0) return i;
	if (first_line) if ((i=executeSubScript(first_line))<0) return i;
	return 0;
}

int HtScript::executeScriptFile(char* filename){
	FILE* f=fopen(filename, FILE_READ);
	if (!f) return HTS_ERR_SCRIPTFILE_NOTFOUND;
	char buf[256];
	int i;
	ReferData script;
	while ((i=fread(buf, 1, 256, f))>0) {
		script.Cat(buf, i);
	}
	return executeScript(script.P, script.L);
}

int HtScript::executeSubScript(ReferLink* lines, ReferLink* def_var, ReferLink** stop_line){
	ReferLink* line = lines;

	ReferLink** args=0;
	int i;
	while (line){
		CurrentEnv.Script = this;
		CurrentEnv.CurrentLine=line;
		CurrentEnv.CurrentDefVar = def_var;
		IsAsynchronousCommand=0;
		IsAsynchronous=0;

//		Log::add("script.log", line->Data, "command:", line->Name.P, __LINE__, __FILE__); 
		args=(ReferLink**) line->Value.P;
		if (line->Name.Cmp("END", 3, false)==0){
			if (stop_line) *stop_line = line;
			//return 0; //?? to be optimized
			line = line->Next;
			break;
		}else if (line->Name.Cmp("NEW", 3, false)==0){
			ReferLink* interf_link = ScriptInterfaces.findName(&args[0]->Name);
			if (!interf_link) return HTS_ERR_NOINTERFACE;
			HtScriptInterface* interf = (HtScriptInterface*)interf_link->Data;
			//create a variable
			char* var_p = (char*) interf->createVariable();
			//add to variable links
			ReferLink* var = 0;
			if (!line->Data){ //not existing variable link
				ReferData interfvar;
				interfvar.Set("@@", 2, true);
				interfvar.Cat(args[0]->Name.P, args[0]->Name.L);
				var = Variables.findName(&interfvar);
				if (!var){
					var=Variables.add(&interfvar, 0, htSC_VARIABLE);
					var->Value.Set(var_p, (long) interf, false);
				}
				line->Data = (long) var;
			}else{ //existing variable link
				var = (ReferLink*) line->Data;
			}
			var->Data = htSC_VARIABLE;
			var->Value.Set(var_p, interf_link->Data, false);
			//def_var=var;
		}else if (line->Name.Cmp("SCRIPT", 6, false)==0){
			ReferLink* var=0;
			ReferLink* interf_link=0;
			var = addInterfaceVariable(&args[0]->Name);
			if (args[1]){
				interf_link = var;
				var=Variables.findName(&args[1]->Name);
				if (!var) var=Variables.add(&args[1]->Name, 0, htSC_POINTER);
				var->Value.Set(interf_link->Value.P, interf_link->Value.L, false);
			}
			/*
			var=Variables.findName(&args[0]->Name);
			if (!var) {
				ReferData name;
				name.Set("@@",2,true);
				name.Cat(args[0]->Name.P, args[0]->Name.L);
				var = Variables.findName(&name);
				if (!var){
					interf_link = ScriptInterfaces.findName(&args[0]->Name);
					if (interf_link){
						HtScriptInterface* interf = (HtScriptInterface*) interf_link->Data;
						char* var_p = (char*) interf->createVariable();
						if (args[1]){
							name.Set("@@",2,true);
							name.Cat(args[1]->Name.P, args[1]->Name.L);
						}
						var=Variables.add(&name, 0, htSC_VARIABLE);
						var->Value.Set(var_p, (long) interf, false);
						def_var = var;
					}else{
						return HTS_ERR_NOINTERFACE;
					}
				}
			}*/
			def_var = var;
			if (!var) return HTS_ERR_NOINTERFACE;

			line = line->Next;
			ReferLink* save_line=line;
			if ((i=executeSubScript(save_line, var, &line))!=0) return i;
//			if (line) line = line->Next;
		}else if (tStrOp::isDigit(line->Name.P[0]) ){
			if (line->Data){
				ReferLink* var = 0;
				var = (ReferLink*) line->Data;
				if (strchr(line->Name.P, '.')){
					var->Data = htSC_DOUBLE;
					double dval=0;
					sscanf(line->Name.P, "%f", (double*) &dval);
					long lval=(long)(10000*dval);
					var->Value.Set((char*) lval, 0, false);
				}else{
					var->Data = htSC_LONG;
					long lval=0;
					sscanf(line->Name.P, "%ld", (long*) &lval);
					var->Value.Set((char*) lval, 0, false);
				}
			}
		}else if (line->Name.P[0]=='\'' || line->Name.P[0]=='"' ){
			ReferLink* var = 0;
			var = (ReferLink*) line->Data;
			var->Data = htSC_STRING;
			var->Value.Set(line->Name.P+1, line->Name.L-2, true);
		}else{
			HtScriptVariableSyntax varsyntax;
			varsyntax.setSentence(line->Name.P, &line->Name.L, false);
			//variable commands;
			if (varsyntax.Type!=QLSyntax::synQL_WORD) return HTS_ERR_SYNTAX_ERROR;

			void* res=0;

			ReferData name;
			HtScriptInterface* interf;
			name.Set(varsyntax.Sentence+varsyntax.Start, varsyntax.StartLen, true);
			ReferLink* val = findVariable(&name); 
			if (!val){
				val = addInterfaceVariable(&name);
			}
				
			if (val && (val->Data==htSC_VARIABLE|| val->Data==htSC_POINTER)
				&& varsyntax.NextType != QLSyntax::synQL_DOT
				){
				if (line->Data){
					ReferLink* var = 0;
					var = (ReferLink*) line->Data;
					var->Data = htSC_POINTER;
					var->Value.Set(val->Value.P, val->Value.L, false);
				} 
			}else if (val && (val->Data==htSC_VARIABLE|| val->Data==htSC_POINTER) 
				&& varsyntax.NextType == QLSyntax::synQL_DOT
				){
				if (val->Data!=htSC_VARIABLE) return HTS_ERR_NOTVARIABLE;
				varsyntax.match();//word;
				varsyntax.match(); //.
				if (varsyntax.Type != QLSyntax::synQL_WORD) return HTS_ERR_SYNTAX_ERROR;
				name.Set(varsyntax.Sentence+varsyntax.Start, varsyntax.StartLen, true);
				interf = (HtScriptInterface*) val->Value.L;
				if (interf->isCommand(name.P)){
					args = (ReferLink**) line->Value.P;

					CurrentEnv.CurrentLine=line;
					CurrentEnv.CurrentDefVar = def_var;
					res = executeCommand(interf, val->Value.P, name.P, args);

				}else return HTS_ERR_NOTCOMMAND;	
			}else if (val) {
				if (line->Data){
					ReferLink* var = 0;
					var = (ReferLink*) line->Data;
					var->Data = val->Data;
					var->Value.Set(val->Value.P, val->Value.L, false);
				}
			}else{
				interf = (HtScriptInterface*) def_var->Value.L;
				if (interf->isCommand(name.P)){
					args = (ReferLink**) line->Value.P;
					CurrentEnv.CurrentLine=line;
					CurrentEnv.CurrentDefVar = def_var;
					res = executeCommand(interf, def_var->Value.P, name.P, args);
				}else return HTS_ERR_NOTCOMMAND;
			}

			if (res && line->Data){
				ReferLink* var = (ReferLink*) line->Data;
				if (var->Data == htSC_VARIABLE && var->Value.L && var->Value.P){
					interf = (HtScriptInterface*) var->Value.L;
					interf->deleteVariable(var->Value.P);
					var->Data = htSC_POINTER;
				}
				var->Value.Set((char*) res, var->Value.L, false);
			}

		}
		line=line->Next;
		if (IsAsynchronousCommand){
			if (!line) break;
			if (line->Name.Cmp("Asynchronous", strlen("Asynchronous"), false)==0){
				line=line->Next;
				IsAsynchronous = 1;
				continue;
			}else
				return 100;
		}
	}
	if (!line){
		if (OnCompleteCall){
			(*OnCompleteCall)(this, OnCompleteCallPara);
			OnCompleteCall = 0;
			OnCompleteCallPara = 0;
		}
	}
	return 0;
}

int HtScript::setOnCompleteCall(void (*fun)(HtScript*, void* ), void* para){
	OnCompleteCall = fun;
	OnCompleteCallPara = para;
	return 0;
}

void* HtScript::executeCommand(HtScriptInterface* interf, void* var, char* command, ReferLink** args){
	void* res=0;

	IsAsynchronousCommand=0;
	if (args){
		for (int i=0; i<4; i++){
			if (args[i]) computeArgValue(args[i]);
			else break;
		}
		res = interf->execCommand(this, var, command, 
			args[0]?args[0]->Value.P:0, 
			args[1]?args[1]->Value.P:0, 
			args[2]?args[2]->Value.P:0, 
			args[3]?args[3]->Value.P:0); 
	} else 
		res = interf->execCommand(this, var, command);

	if (interf->IsAsynchronous){
		IsAsynchronousCommand = 1;
		interf->IsAsynchronous = 0;
	}

	return res;
}

int HtScript::computeArgValue(ReferLink* arg){
	ReferLink* val=0;
	if (!arg || !arg->Name.P ) return 0;
	if (tStrOp::isDigit(arg->Name.P[0])){
		long i=0;
		while (tStrOp::isDigit(arg->Name.P[i])) i++;
		if (arg->Name.P[i]=='.'){
			arg->Data = htSC_DOUBLE;
			double dval=0;
			sscanf(arg->Name.P, "%lf", (double*) &dval);
			long lval=(long)(10000*dval);
			arg->Value.Set((char*) lval, 0, false);
		}else{
			arg->Data = htSC_LONG;
			long lval=0;
			sscanf(arg->Name.P, "%ld", (long*) &lval);
			arg->Value.Set((char*) lval, 0, false);
		}
	}else if (arg->Name.P[0]=='\''||arg->Name.P[0]=='"'){
		arg->Value.Set(arg->Name.P+1, arg->Name.L-2, true);
		arg->Data = htSC_STRING;
	}else if ( val=findVariable(&arg->Name) ) {
		if (arg->Data == htSC_VARIABLE && arg->Value.L && arg->Value.P){
			HtScriptInterface* interf = (HtScriptInterface*) arg->Value.L;
			interf->deleteVariable(arg->Value.P);
			arg->Data = htSC_POINTER;
		}

		if (val->Data == htSC_STRING){
			arg->Value.Set(val->Value.P, strlen(val->Value.P), true);
			arg->Data = htSC_STRING;
		}else if (val->Data == htSC_LONG) {
			arg->Value.Set(val->Value.P, 0, false);
			arg->Data = htSC_LONG;
		}else if (val->Data == htSC_DOUBLE){
			arg->Value.Set(val->Value.P, 0, false);
			arg->Data = htSC_DOUBLE;
		}else{
			arg->Value.Set(val->Value.P, val->Value.L, false);
			arg->Data = htSC_POINTER;
		}
	}else 
		return HTS_ERR_VARIABLE_NOTFOUND;

	return 0;
}

ReferLink* HtScript::getVariable(ReferData* varname){
	ReferLink* var = findVariable(varname);
//	if (var) computeArgValue(var);
	return var;
}

ReferLink* HtScript::getDefVariable(){
	computeArgValue(CurrentEnv.CurrentDefVar);
	return CurrentEnv.CurrentDefVar;
}

int HtScript::setVariable(ReferData* varname, ReferData* varvalue){
	ReferLink* var = newVariableLink(varname);
	var->Value.Set(varvalue->P, varvalue->L, true);
	var->Data = htSC_STRING;
	return 0;
}

ReferLink* HtScript::findVariable(ReferData* varname){
	ReferLink* val = Variables.findName(varname);
	if (!val) {
		ReferData interfname;
		interfname.Set("@@",2, true);
		interfname.Cat(varname->P, varname->L);
		val = Variables.findName(&interfname);
	}
	return val;
}

ReferLink* HtScript::newVariableLink(ReferData* varname){
	ReferLink* var = Variables.findName(varname);
	if (!var){
		var=Variables.add(varname, 0, 0);
	}
	return var;
}

ReferLink* HtScript::addInterfaceVariable(ReferData* varname){
	ReferLink* var=0;
	var=Variables.findName(varname);
	if (!var) {
		ReferData name;
		name.Set("@@",2,true);
		name.Cat(varname->P, varname->L);
		var = Variables.findName(&name);
		if (!var){
			ReferLink* interf_link;
			interf_link = ScriptInterfaces.findName(varname);
			if (interf_link){
				HtScriptInterface* interf = (HtScriptInterface*) interf_link->Data;
				char* var_p = (char*) interf->createVariable();
				var=Variables.add(&name, 0, htSC_VARIABLE);
				var->Value.Set(var_p, (long) interf, false);
			}else{
				return 0;
			}
		}
	}
	return var;
}

ReferLink* HtScript::newScriptLine(){
	ReferLink* line = new ReferLink;
	if (LastScriptLine) LastScriptLine->Next = line;
	else if (ScriptLines){
		for (LastScriptLine=ScriptLines; LastScriptLine->Next; LastScriptLine=LastScriptLine->Next);
		LastScriptLine->Next = line;
	}else{
		ScriptLines = line;
	}
	LastScriptLine = line;
	return line;
}

ReferLink* HtScript::findCommandContext(ReferData* command){
	ReferLink* context = ScriptContext;
	HtScriptInterface* interf;
	while (context){
		interf = (HtScriptInterface*) context->Value.L;
		if (interf->isCommand(command->P)) break;
		context = context->Next;
	}
	return context;
}

ReferLink* HtScript::addCommandContext(ReferLink* var){
	ReferLink* context = new ReferLink;
	context->Name.Set(var->Name.P, var->Name.L, true);
	context->Value.Set(var->Value.P, var->Value.L, false);
	context->Next = ScriptContext;
	ScriptContext = context;
	return context;
}

int HtScript::removeCommandContext(ReferLink* var){
	ReferLink** context = &ScriptContext;
	while (*context){
		if ((*context)->Name.Cmp(&var->Name, false)==0) break;
		context = &(*context)->Next;
	}
	if (*context){
		ReferLink* tmpcon=*context;
		*context = (*context)->Next;
		tmpcon->Next = 0;
		delete tmpcon;
		return 1;
	}
	return 0;
}

//==================================================

#include "htbrowser.h"

HtBrowserScriptInterface::HtBrowserScriptInterface(){
	InterfaceName.Set("HtBrowser", strlen("HtBrowser"), true);
}

HtBrowserScriptInterface::~HtBrowserScriptInterface(){
	reset();
}

int HtBrowserScriptInterface::isCommand(char* command){
	char* support[]={
		"setUrl",
		"getUrl",
		"changeUrl",
		"setVariable",
		"setMethod",
		"useForm",
		"setCookie",
		"navigate",
		"click",
		"saveFileAll", 
		"getContentData",
		"printResultData",
		"formatHtmlResult",
		"getContentLength",
		"setQuery",
		"dotQuery",
		"isEOF",
		"moveFirst",
		"moveNext",
		"getValue",
		"getValueByIndex",
		"getValueByName",
		"getFieldsCount",
		"getFieldName",
		"getHtql",
		0
	};
	for (int i=0; support[i]; i++){
		if (tStrOp::strNcmp(command, support[i],strlen(command)+1,false)==0)
			return true;
	}
	return false;
}

void* HtBrowserScriptInterface::execCommand(HtScript* callfrom, void* var, char* command, void* arg1, void* arg2, void* arg3, void* arg4){
	if (!var || !command) return 0;
	void* ret=0;
	HtBrowser* v = (HtBrowser*) var;
	if (tStrOp::strNcmp(command, "setUrl",strlen(command)+1,false)==0){
		ret= (void*) v->setUrl((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);

	}else if (tStrOp::strNcmp(command, "getUrl",strlen(command)+1,false)==0){
		ret=  (void*) v->Url.P;
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_STRING);

	}else if (tStrOp::strNcmp(command, "changeUrl",strlen(command)+1,false)==0){
		ret=  (void*) v->Url.Set((char*) arg1, strlen((const char*) arg1), true);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);

	}else if (tStrOp::strNcmp(command, "setVariable",strlen(command)+1,false)==0){
		ret=  (void*) v->setVariable((const char*) arg1, (const char*) arg2);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING, arg2, htSC_STRING);

	}else if (tStrOp::strNcmp(command, "useForm",strlen(command)+1,false)==0){
		ret=  (void*) v->useForm((int)(long)arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_LONG);

	}else if (tStrOp::strNcmp(command, "setMethod",strlen(command)+1,false)==0){
		ret=  (void*) v->setMethod((int)(long)arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_LONG);

	}else if (tStrOp::strNcmp(command, "setCookie",strlen(command)+1,false)==0){
		ret=  (void*) v->setCookie((const char*) arg1, (const char*) arg2);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING, arg2, htSC_STRING);

	}else if (tStrOp::strNcmp(command, "navigate",strlen(command)+1,false)==0){
		ret=  (void*) v->navigate();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW);
	}else if (tStrOp::strNcmp(command, "click",strlen(command)+1,false)==0){
		ret=  (void*) v->clickItem((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW);
	}else if (tStrOp::strNcmp(command, "saveFileAll",strlen(command)+1,false)==0){
		ret=  (void*) v->Html.saveFileAll((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "getContentData",strlen(command)+1,false)==0){
		ret=  (void*) v->Html.getContentData();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW);
	}else if (tStrOp::strNcmp(command, "getContentLength",strlen(command)+1,false)==0){
		ret=  (void*) v->Html.getContentLength();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "setQuery",strlen(command)+1,false)==0){
		ret=  (void*) v->setQuery((const char*) arg1, 0);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "dotQuery",strlen(command)+1,false)==0){
		ret=  (void*) v->dotQuery((const char*) arg1, 0);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "isEOF",strlen(command)+1,false)==0){
		ret=  (void*) v->isEOF();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "moveFirst",strlen(command)+1,false)==0){
		ret=  (void*) v->moveFirst();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "moveNext",strlen(command)+1,false)==0){
		ret=  (void*) v->moveNext();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "getValue",strlen(command)+1,false)==0){
		ret=  (void*) v->getValue((int)(long)arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "getValueByIndex",strlen(command)+1,false)==0){
		ret=  (void*) v->getValue((int)(long)arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "getValueByName",strlen(command)+1,false)==0){
		ret=  (void*) v->getValue((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "getFieldsCount",strlen(command)+1,false)==0){
		ret=  (void*) v->getFieldsCount();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "getFieldName",strlen(command)+1,false)==0){
		ret=  (void*) v->getFieldName((int)(long) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_STRING, arg1, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "getHtql",strlen(command)+1,false)==0){
		ret=  (void*) &v->htql;
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW);
	}else if (tStrOp::strNcmp(command, "formatHtmlResult",strlen(command)+1,false)==0){
		ret=  (void*) v->htql.Parser->formatHtmlResult((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "printResultData",strlen(command)+1,false)==0){
		ret=  (void*) v->htql.Parser->printResultData((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}
	return ret;
}

void* HtBrowserScriptInterface::createVariable(){
	return new HtBrowser;
}
void HtBrowserScriptInterface::deleteVariable(void* var){
	HtBrowser* v=(HtBrowser*) var;
	delete v;
}


//==================================================

#include "htmlql.h"

HtHtqlScriptInterface::HtHtqlScriptInterface(){
	InterfaceName.Set("HTQL", strlen("HTQL"), true);
}

HtHtqlScriptInterface::~HtHtqlScriptInterface(){
	reset();
}

int HtHtqlScriptInterface::isCommand(char* command){
	char* support[]={
		"setUrl",
		"setSourceData",
		"setSourceUrl",
		"setVariable",
		"setUrlParameter",
		"setCookie",
		"saveFileAll", 
		"printResultData",
		"getContentData",
		"getContentLength",
		"setGlobalVariable",
		"setQuery",
		"dotQuery",
		"isEOF",
		"moveFirst",
		"moveNext",
		"getValue",
		"getValueByIndex",
		"getValueByName",
		"getFieldsCount",
		"getFieldName",
		"addHtqlResult",
		"formatHtmlResult",
		"printValue",
		"printTableHtml",
		0
	};
	for (int i=0; support[i]; i++){
		if (tStrOp::strNcmp(command, support[i],strlen(command)+1,false)==0)
			return true;
	}
	return false;
}
void* HtHtqlScriptInterface::execCommand(HtScript* callfrom, void* var, char* command, void* arg1, void* arg2, void* arg3, void* arg4){
	if (!var || !command) return 0;
	HtmlQL* v = (HtmlQL*) var;
	void* ret=0;
	if (tStrOp::strNcmp(command, "setUrl",strlen(command)+1,false)==0){
		ret= (void*) v->setUrl((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "setSourceData",strlen(command)+1,false)==0){
		ret= (void*) v->setSourceData((const char*) arg1, strlen((const char*) arg1), (int)(long) arg2);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "setSourceUrl",strlen(command)+1,false)==0){
		ret= (void*) v->setSourceUrl((const char*) arg1, strlen((const char*) arg1) );
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "setVariable",strlen(command)+1,false)==0){
		ret= (void*) v->setUrlParameter((const char*) arg1, (const char*) arg2);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING, arg2, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "setUrlParameter",strlen(command)+1,false)==0){
		ret= (void*) v->setUrlParameter((const char*) arg1, (const char*) arg2);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING, arg2, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "setCookie",strlen(command)+1,false)==0){
		ret= (void*) v->Html->setCookie((const char*) arg1, (const char*) arg2);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING, arg2, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "saveFileAll",strlen(command)+1,false)==0){
		ret= (void*) v->Html->saveFileAll((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "printResultData",strlen(command)+1,false)==0){
		ret= (void*) v->Parser->printResultData((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "getContentData",strlen(command)+1,false)==0){
		ret= (void*) v->Html->getContentData();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW);
	}else if (tStrOp::strNcmp(command, "getContentLength",strlen(command)+1,false)==0){
		ret= (void*) v->Html->getContentLength();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "setGlobalVariable",strlen(command)+1,false)==0){
		ret= (void*) v->setGlobalVariable((const char*) arg1, (const char*) arg2);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING, arg2, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "setQuery",strlen(command)+1,false)==0){
		ret= (void*) v->setQuery((const char*) arg1, 0);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "dotQuery",strlen(command)+1,false)==0){
		ret= (void*) v->dotQuery((const char*) arg1, 0);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "isEOF",strlen(command)+1,false)==0){
		ret= (void*) v->isEOF();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "moveFirst",strlen(command)+1,false)==0){
		ret= (void*) v->moveFirst();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "moveNext",strlen(command)+1,false)==0){
		ret= (void*) v->moveNext();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "getValueByIndex",strlen(command)+1,false)==0){
		ret= (void*) v->getValue((int)(long)arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "getValue",strlen(command)+1,false)==0){
		ret= (void*) v->getValue((int)(long)arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "getValueByName",strlen(command)+1,false)==0){
		ret= (void*) v->getValue((const char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "getFieldsCount",strlen(command)+1,false)==0){
		ret= (void*) v->getFieldsCount();
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "getFieldName",strlen(command)+1,false)==0){
		ret= (void*) v->getFieldName((int)(long) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_STRING, arg1, htSC_LONG);
	}else if (tStrOp::strNcmp(command, "addHtqlResult",strlen(command)+1,false)==0){
		if (arg1) 
			ret= (void*) v->Parser->copyAllRowsSetItem(((HTQL*) arg1)->Parser->Data, &v->Parser->Data);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_UNKNOW, arg1, htSC_UNKNOW);
	}else if (tStrOp::strNcmp(command, "formatHtmlResult",strlen(command)+1,false)==0){
		if (arg1) 
			ret= (void*) v->Parser->formatHtmlResult((char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG, arg1, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "printValue",strlen(command)+1,false)==0){
		int num=1;
		char* p;
		ret= (void*) v->getValue((int)(long)arg1);
		p=(char*) ret;
		if (!arg2 || !((char*) arg2)[0]){
			printf("%s", p);
		}else{
			ReferData val;
			val.Set(p, strlen(p), false);
			val.saveFile((char*) arg2);
		}
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG, arg1, htSC_LONG, arg2, htSC_STRING);
	}else if (tStrOp::strNcmp(command, "printTableHtml",strlen(command)+1,false)==0){
		if (arg1) 
			ret= (void*) v->Parser->formatHtmlResult((char*) arg1);
		addLog(SCRIPT_LOG_FILE, __FILE__, __LINE__, command, ret, htSC_LONG, arg1, htSC_STRING);
	}
	return ret;
}

void* HtHtqlScriptInterface::createVariable(){
	return new HtmlQL;
}
void HtHtqlScriptInterface::deleteVariable(void* var){
	HtmlQL* v=(HtmlQL*) var;
	delete v;
}

//==============================================================

HtStringScriptInterface::HtStringScriptInterface(){
	InterfaceName.Set("String", strlen("String"), true);
}

HtStringScriptInterface::~HtStringScriptInterface(){
	reset();
}

int HtStringScriptInterface::isCommand(char* command){
	char* support[]={
		"set",
		"get",
		"cat",
		"strtime",
		"print",
		"log",
		0
	};
	for (int i=0; support[i]; i++){
		if (tStrOp::strNcmp(command, support[i],strlen(command)+1,false)==0)
			return true;
	}
	return false;
}
void* HtStringScriptInterface::execCommand(HtScript* callfrom, void* var, char* command, void* arg1, void* arg2, void* arg3, void* arg4){
	if (!var || !command) return 0;
	ReferData* v = (ReferData*) var;
	if (tStrOp::strNcmp(command, "set",strlen(command)+1,false)==0){
		if (arg1) v->Set((char*) arg1, strlen((char*) arg1), true);
		return (void*) v->P;
	}else if (tStrOp::strNcmp(command, "get",strlen(command)+1,false)==0){
		return (void*) v->P;
	}else if (tStrOp::strNcmp(command, "cat",strlen(command)+1,false)==0){
		if (arg1) v->Cat((char*) arg1, strlen((char*) arg1));
		if (arg2) v->Cat((char*) arg2, strlen((char*) arg2));
		if (arg3) v->Cat((char*) arg3, strlen((char*) arg3));
		if (arg4) v->Cat((char*) arg4, strlen((char*) arg4));
		return (void*) v->P;
	}else if (tStrOp::strNcmp(command, "strtime",strlen(command)+1,false)==0){
		char buf[40];
		sprintf(buf, "%ld", time(NULL));
		v->Set(buf, strlen(buf), true);
		return (void*) v->P;
	}else if (tStrOp::strNcmp(command, "print",strlen(command)+1,false)==0){
		if (v->P) printf("%s", v->P);
		if (arg1) printf("%s", (char*) arg1);
		if (arg2) printf("%s", (char*) arg2);
		if (arg3) printf("%s", (char*) arg3);
		if (arg4) printf("%s", (char*) arg4);
		return (void*) v->P;
	}else if (tStrOp::strNcmp(command, "log",strlen(command)+1,false)==0){
		Log::add((char*) arg1, 0, (char*) v->P, (char*) arg2, 0, (char*) arg3);
		return (void*) v->P;
	}
	return 0;
}

void* HtStringScriptInterface::createVariable(){
	return new ReferData;
}
void HtStringScriptInterface::deleteVariable(void* var){
	ReferData* v=(ReferData*) var;
	delete v;
}

//==============================================================

HtBrowserScriptInterface HtBrowserScript::BrowserInteface;
HtHtqlScriptInterface HtBrowserScript::HtqlScriptInterface;
HtStringScriptInterface HtBrowserScript::StringScriptInterface;

HtBrowserScript::HtBrowserScript(){
	registerInterface(&BrowserInteface, &BrowserInteface.InterfaceName);
	registerInterface(&HtqlScriptInterface, &HtqlScriptInterface.InterfaceName);
	registerInterface(&StringScriptInterface, &StringScriptInterface.InterfaceName);
}

HtBrowserScript::~HtBrowserScript(){
	reset();
}

void HtBrowserScript::reset(){
	BrowserInteface.reset();
	registerInterface(&BrowserInteface, &BrowserInteface.InterfaceName);
	registerInterface(&HtqlScriptInterface, &HtqlScriptInterface.InterfaceName);
	registerInterface(&StringScriptInterface, &StringScriptInterface.InterfaceName);
}
