#include "htscriptgen.h"
#include "htscript.h"
#include "referdata.h"
#include "htql.h"
#include "stroper.h"
#include "stdio.h"
#include "string.h"
#include "stack.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HtScriptGen::HtScriptGen(){
	
}

HtScriptGen::~HtScriptGen(){
	reset();
}
void HtScriptGen::reset(){
	script.reset();
}

int HtScriptGen::generateWebView(char* url, int form_index, tStack* values, ReferData* result, ReferData* result_table){
	ReferData str;
	char buf[256];
	*result = "";

	//navigate to the form page
	str="Script HtBrowser a;\n";
	sprintf(buf, "SetUrl \"%s\";\n", url);
	str+=buf;
	str+="navigate;\n";
	sprintf(buf, "setQuery \"<form>%d:tn &form_inputs .<form_inputs>.<form_input>{name=:name; id=:id; value=:value; tag=:tag; type=:type; desc=:tx;}\";\n", form_index);
	str+=buf;
	str+="htql=getHtql;\n";
	str+="End Script;\n";

	int i=script.executeScript(str.P, str.L);
	ReferData name;
	name = "htql";
	ReferLink* link=script.getVariable(&name);
	HTQL* ql=(HTQL*) link->Value.P;

	//check which fields are to be filled
	ReferData value;
	ReferData tag;
	ReferData type;
	char* val;
	tStack new_values;
	new_values.Type = tStack::ordFIFO;
	int cur_val=1;
	while (!ql->isEOF()){
		name = ql->getValue("name");
		if (!name.L) name=ql->getValue("id"); 
		value = ql->getValue("value");
		tag = ql->getValue("tag");
		type = ql->getValue("type");

		if (values->search(name.P, &val)){
			new_values.set(0, name.P, val);
		}else if ( !tag.Cmp("input", 5, false) && (!type.Cmp("",0,false) || !type.Cmp("text",4,false) || !type.Cmp("passwd",6,false))){
			int k=0;
			for (tStack* st=values->Next; st; st=st->Next){
				if (!st->Key[0]){
					k++;
					if (k==cur_val){
						cur_val++;
						new_values.set(0, name.P, st->Value);
						break;
					}
				}
			}
		}
//		printf("%s, %s, %s, %s\n", name.P?name.P:"", ql->getValue("value"), ql->getValue("tag"), ql->getValue("type"));
		ql->moveNext();
	}

	//submit the form and retrieve the repeat view
	str="Script HtBrowser a;\n";
	sprintf(buf, "UseForm %d;\n", form_index);
	str += buf;
	tStack* st;
	for (st=new_values.Next; st; st=st->Next){
		sprintf(buf, "SetVariable \"%s\", \"%s\";\n", st->Key, st->Value);
		str += buf;
//		printf("%s=%s\n", st->Key, st->Value);
	}
//	str+="setMethod 0;\n";
	str+="navigate;\n";
	sprintf(buf, "setQuery \"&find_repeat_views(5) .<find_repeat_views>.<find_view>:htql, candidate, view_items\";\n");
	str+=buf;
	str+="ql=getHtql;\n";
	str+="End Script;\n";
	script.executeScript(str.P, str.L);

	name = "ql";
	link=script.getVariable(&name);
	ql=(HTQL*) link->Value.P;
	value = ql->getValue(1);

	if (value.P){
	//	printf("%s\n", value.P);
		ql->setQuery(value.P);
	//	ql->Parser->formatHtmlResult("c:\\result.html");
	}

	//compose the result script
	str="Script HtBrowser a;\n";
	sprintf(buf, "SetUrl \"%s\";\n", url);
	str+=buf;
	str+="navigate;\n";
	sprintf(buf, "UseForm %d;\n", form_index);
	str += buf;
	int para=0;
	for (st=new_values.Next; st; st=st->Next){
		//sprintf(buf, "SetVariable \"%s\", \"%s\";\n", st->Key, st->Value);
		sprintf(buf, "SetVariable \"%s\", para%d;\n", st->Key, ++para);
		str += buf;
	//	printf("%s=%s\n", st->Key, st->Value);
	}
	str+="navigate;\n";
	str +="setQuery '";
	str += value;
	str += "';\n";
	str += "ql = getHTQL();\n";
	str+="End Script;\n";
	//printf("\n--Script---\n%s\n", str.P);

	*result = str;


	if (result_table){
		str = "<Table Name='webview' DataSource='webview' Htql='' HtqlVar='ql'>\n";
		int outfields = ql->getFieldsCount();
		sprintf(buf, "\t<Fields FieldsNum='%i' InFields='%i' OutFields='%i'>\n", para+outfields, para, outfields);
		str+= buf;
		i=0;
		for (st=new_values.Next; st; st=st->Next){
			sprintf(buf, "\t\t<Field Name='PARA%d' Type='varchar' Length='100' Precision=''>\n", ++i);
			str += buf;
			sprintf(buf, "\t\t\t<ScriptField Type='IN' Name='para%d' Value='", i);
			str += buf;
			str += st->Value;
			str += "'></ScriptField>\n";
			str += "\t\t</Field>\n";
		}
		for (i=1; i<=outfields; i++){
			sprintf(buf, "\t\t<Field Name='COLUMN%d' Type='memo' Length='250' Precision=''>\n", para+i);
			str += buf;
			sprintf(buf, "\t\t\t<ScriptField Type='OUT' Htql='%%%d'></ScriptField>\n", i);
			str += buf;
			str += "\t\t</Field>\n";
		}
		str += "\t</Fields>\n</Table>\n";

		result_table->Set(str.P, str.L, false);
		result_table->setToFree(true);
		str.setToFree(false);
	}

	return 0;
}

int HtScriptGen::generateHtmlTable(char* url, char* candiate_htql, ReferData* result, ReferData* result_table){
	ReferData str;
	char buf[256];

	str="Script HtBrowser a;\n";
	sprintf(buf, "SetUrl \"%s\";\n", url);
	str+=buf;
	str+="navigate;\n";
	if (!candiate_htql || !*candiate_htql) {
		str += "setQuery \"&find_repeat_views(5) .<find_repeat_views>.<find_view>:htql, candidate, view_items\";\n";
	}else{
		str += "setQuery \"";
		str += candiate_htql;
		str += " &find_best_views(5) .<find_best_views>.<find_view>:htql, candidate, view_items\";\n";
	}
	str+="ql=getHtql;\n";
	str+="End Script;\n";

	//navigate to the html page
	ReferData value;
	int i=script.executeScript(str.P, str.L);
	ReferData name;
	name = "ql";
	ReferLink* link=script.getVariable(&name);
	HTQL* ql=(HTQL*) link->Value.P;
	value = ql->getValue(1);

	if (value.P){
	//	printf("%s\n", value.P);
		ql->setQuery(value.P);
	//	ql->Parser->formatHtmlResult("c:\\result.html");
	}

	//compose the result script
	str="Script HtBrowser a;\n";
	sprintf(buf, "SetUrl \"%s\";\n", url);
	str+=buf;
	str+="navigate;\n";
	str +="setQuery '";
	str += value;
	str += "';\n";
	str += "ql = getHTQL();\n";
	str+="End Script;\n";
	//printf("\n--Script---\n%s\n", str.P);

	*result = str;

	if (result_table){
		tStrOp::replaceInplace(value.P, "\r\n", " ");
		tStrOp::replaceInplace(value.P, "\n", " ");
		str = "<Table Name='htmltable' DataSource='htmltable' Htql='";
		str += value.P;
		str += "'>\n";
		int outfields = ql->getFieldsCount();
		sprintf(buf, "\t<Fields FieldsNum='%i'>\n", outfields);
		str+= buf;
		i=0;
		for (i=1; i<=outfields; i++){
			sprintf(buf, "\t\t<Field Name='COLUMN%d' Type='memo' Length='250' Precision=''>\n", i);
			str += buf;
			sprintf(buf, "\t\t\t<Htql Query='%%%d'></Htql>\n", i);
			str += buf;
			str += "\t\t</Field>\n";
		}
		str += "\t</Fields>\n</Table>\n";

		result_table->Set(str.P, str.L, false);
		result_table->setToFree(true);
		str.setToFree(false);
	}

	return 0;
}
