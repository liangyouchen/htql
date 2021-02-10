#include "htqlupdate.h"

#include "qhtql.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


HTQLFunctionUpdateStruct::HTQLFunctionUpdateStruct(){
	LastOffset=0;
	ToUpdateSource=true;
};

HTQLFunctionUpdateStruct::~HTQLFunctionUpdateStruct(){
	reset();
}

void HTQLFunctionUpdateStruct::reset(){
	UpdatedData.reset();
	LastOffset=0;
	ToUpdateSource=true;
}

int HTQLFunctionUpdateStruct::addUpdateFunction(HTQLParser* parser, int action, char* funname, char* description){//&insert(value);
	HTQLFunction* update_function=0;
	parser->addHtqlFunction(funname, (void*) functionUpdate, description, &update_function);

	if (update_function) {
		HTQLFunctionUpdateStruct* update = new HTQLFunctionUpdateStruct;
		update->Action = action;
		update_function->FunData = (HTQLFunctionUpdateStruct*) update;
		update_function->FunPrepare = functionUpdateFunPrepare;
		update_function->FunComplete = functionUpdateFunComplete;
		update_function->FunFinalRelease = functionUpdateFunFinalRelease;
	}
	return 0;
}

int HTQLFunctionUpdateStruct::functionUpdate(char*p, void* call_from, ReferData* tx){ //&insert(value)
	HTQLParser* call = (HTQLParser* )call_from;
	HTQLFunctionUpdateStruct* update = (HTQLFunctionUpdateStruct*) call->CurrentFunction->FunData;
	char* value=0, *name=0, *attr=0, *attr_value=0;
	ReferData replace;
	unsigned int len=0, len1=0;
	long item_len=0;
	char quote='"';
	int have_quote=false;
	ReferLink* para=0;
	if (call->FunctionCurrentItem->SourceOffset >= (unsigned long)update->LastOffset){
		switch (update->Action){
		case funINSERT_AFTER:
			if (call->FunctionCurrentItem->E.P >= call->FunctionCurrentItem->S.P){
				item_len = (call->FunctionCurrentItem->E.P - call->FunctionCurrentItem->S.P) + call->FunctionCurrentItem->E.L;
			}else{
				item_len = call->FunctionCurrentItem->Data.L;
			}
		case funINSERT_BEFORE:
			update->UpdatedData.Cat(call->SourceData.P+update->LastOffset, call->FunctionCurrentItem->SourceOffset-update->LastOffset + item_len);
			if (call->FunctionParameters && call->FunctionParameters->Value.P ){
				update->UpdatedData.Cat(call->FunctionParameters->Value.P, call->FunctionParameters->Value.L);
				value = call->FunctionParameters->Value.P;
			}
			update->LastOffset = call->FunctionCurrentItem->SourceOffset + item_len;
			break;

		case funUPDATE:
			//item_len is the total length of item to replace including S and E
			if (call->FunctionCurrentItem->E.P >= call->FunctionCurrentItem->S.P){
				item_len = (call->FunctionCurrentItem->E.P - call->FunctionCurrentItem->S.P) + call->FunctionCurrentItem->E.L;
				replace.Set(call->FunctionCurrentItem->S.P, item_len, true);
			}else{
				item_len = call->FunctionCurrentItem->Data.L;
				replace.Set(call->FunctionCurrentItem->Data.P, item_len, true);
			}
			//cat new item
			para=call->FunctionParameters;
			if (para && para->Value.P){
				if (para->Next && para->Next->Value.P){
					update->ToUpdateSource=false;
					//exact replace, change to match replace later
					value=tStrOp::replaceMalloc(replace.P, para->Value.P, para->Next->Value.P);
					if (value){
						call->FunctionCurrentItem->Data.Set(value, strlen(value), false);
						call->FunctionCurrentItem->Data.setToFree(true);
						update->UpdatedData.Cat(value, strlen(value));
					}else{
						call->FunctionCurrentItem->Data.Set("",0,true);
					}
					call->FunctionCurrentItem->E.Set(call->FunctionCurrentItem->Data.P+call->FunctionCurrentItem->Data.L, 0, false);
					call->FunctionCurrentItem->S.Set(call->FunctionCurrentItem->Data.P, 0, false);
					value=0;
				}else {
					//cat text before this item
					update->UpdatedData.Cat(call->SourceData.P+update->LastOffset, call->FunctionCurrentItem->SourceOffset-update->LastOffset);

					update->UpdatedData.Cat(para->Value.P, para->Value.L);
					value = para->Value.P;
				}
			}
			update->LastOffset = call->FunctionCurrentItem->SourceOffset + item_len;
			break;
			
		case funDELETE:
			if (call->FunctionCurrentItem->E.P >= call->FunctionCurrentItem->S.P){
				item_len = (call->FunctionCurrentItem->E.P - call->FunctionCurrentItem->S.P) + call->FunctionCurrentItem->E.L;
			}else{
				item_len = call->FunctionCurrentItem->Data.L;
			}
			update->UpdatedData.Cat(call->SourceData.P+update->LastOffset, call->FunctionCurrentItem->SourceOffset-update->LastOffset);
			update->LastOffset = call->FunctionCurrentItem->SourceOffset + item_len;
			break;
		case funSET_ATTRIBUTE:
			if (call->FunctionCurrentItem->E.P >= call->FunctionCurrentItem->S.P){
				item_len = (call->FunctionCurrentItem->E.P - call->FunctionCurrentItem->S.P) + call->FunctionCurrentItem->E.L;
			}else{
				item_len = call->FunctionCurrentItem->Data.L;
			}
			if (call->FunctionParameters && call->FunctionParameters->Value.L){
				name=call->FunctionParameters->Value.P;
				if (call->FunctionParameters->Next&& call->FunctionParameters->Next->Value.L){
					value=call->FunctionParameters->Next->Value.P;
				}
			}

			if (!name) name="";
			if (!value) value="";
			if (!strchr(value, '"')) quote='"';
			else if (!strchr(value,'\'')) quote='\'';
			else{
				tStrOp::replaceInplace(value, "\"", "'");
				quote='"';
			}
			replace.Malloc(strlen(name)+strlen(value)+6);
			sprintf(replace.P, " %s=%c%s%c", name, quote, value, quote);
			
			if (TagOperation::isTag(call->FunctionCurrentItem->S.P)){
				attr=TagOperation::targetAttribute(call->FunctionCurrentItem->S.P, name, &len);
				if (attr){
					len=TagOperation::getAttributeLength(attr);
					/*attr_value=TagOperation::targetValue(attr, &len1, &have_quote);
					if (attr_value){
						len=(attr_value-attr)+len1;
						if (have_quote) len++;
					}*/
					if (*(attr-1)==' '){
						attr--; 
						len++;
					}
				}else{
					len=TagOperation::getTagLength(call->FunctionCurrentItem->S.P);
					if (len>0 && call->FunctionCurrentItem->S.P[len-1]=='>'){
						if (len>1 && call->FunctionCurrentItem->S.P[len-2]=='/')
							attr=call->FunctionCurrentItem->S.P+(len-2);
						else
							attr=call->FunctionCurrentItem->S.P+(len-1);
						len=0;
					}else{
						attr=TagOperation::getLabel(call->FunctionCurrentItem->S.P, &len);
						attr+=len;
						len=0;
					}
				}
				update->UpdatedData.Cat(call->SourceData.P+update->LastOffset, 
					call->FunctionCurrentItem->SourceOffset-update->LastOffset+(attr-call->FunctionCurrentItem->S.P)
					);
				update->UpdatedData+=replace.P;

				update->LastOffset = call->FunctionCurrentItem->SourceOffset + (attr-call->FunctionCurrentItem->S.P)+len;
			}
			break;		
		case funDELETE_ATTRIBUTE:
			if (call->FunctionCurrentItem->E.P >= call->FunctionCurrentItem->S.P){
				item_len = (call->FunctionCurrentItem->E.P - call->FunctionCurrentItem->S.P) + call->FunctionCurrentItem->E.L;
			}else{
				item_len = call->FunctionCurrentItem->Data.L;
			}
			if (call->FunctionParameters && call->FunctionParameters->Value.L){
				name=call->FunctionParameters->Value.P;
			}

			if (!name) name="";
			
			if (TagOperation::isTag(call->FunctionCurrentItem->S.P)){
				attr=TagOperation::targetAttribute(call->FunctionCurrentItem->S.P, name, &len);
				if (attr){
					len=TagOperation::getAttributeLength(attr);
					if (*(attr-1)==' '){
						attr--; 
						len++;
					}
					update->UpdatedData.Cat(call->SourceData.P+update->LastOffset, 
						call->FunctionCurrentItem->SourceOffset-update->LastOffset+(attr-call->FunctionCurrentItem->S.P)
						);

					update->LastOffset = call->FunctionCurrentItem->SourceOffset + (attr-call->FunctionCurrentItem->S.P)+len;
				}
			}
			break;		
		}
	}
	if (value) {
		*tx=value;
		return 1;
	}else{
		return 0;
	}
}
int HTQLFunctionUpdateStruct::functionUpdateFunPrepare(char* p, void* call_from, ReferData* tx){
	HTQLParser* call = (HTQLParser* )call_from;
	HTQLFunctionUpdateStruct* update = (HTQLFunctionUpdateStruct*) call->CurrentFunction->FunData;
	update->UpdatedData.reset();
	update->LastOffset = 0;
	return 0;
}
int HTQLFunctionUpdateStruct::functionUpdateFunComplete(char* p, void* call_from, ReferData* tx){
	HTQLParser* call = (HTQLParser* )call_from;
	HTQLFunctionUpdateStruct* update = (HTQLFunctionUpdateStruct*) call->CurrentFunction->FunData;
	ReferLink* para=call->FunctionParameters;
	if (update->Action == funUPDATE  && para && para->Next && para->Next->Value.P){
		update->ToUpdateSource = false;
	}
	if (update->ToUpdateSource){
		if (update->LastOffset < call->SourceData.L){
			update->UpdatedData.Cat(call->SourceData.P+update->LastOffset, call->SourceData.L-update->LastOffset);
		}
		call->SourceData.Set(update->UpdatedData.P, update->UpdatedData.L, false);
		call->SourceData.setToFree(true);
		update->UpdatedData.setToFree(false);

		update->UpdatedData.reset();
		update->LastOffset=0;

		call->resetData();
	}

	return 0;
}

void HTQLFunctionUpdateStruct::functionUpdateFunFinalRelease(HTQLFunction* fun){
	if (fun->FunData){
		delete (HTQLFunctionUpdateStruct*) fun->FunData;
		fun->FunData = 0;
	}
}

