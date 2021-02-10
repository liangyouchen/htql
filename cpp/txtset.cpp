// LASTMODIFY CLY20000903
#include "txtset.h"
#include <malloc.h>
#include <string.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


TxtSet::TxtSet(){
	m_RecordCount = 0; 
	m_RecordIndex = 0;
	m_OpenFlag=ofCLOSE;
	m_Move = NULL;
}

TxtSet::~TxtSet(){
}

int TxtSet::closeSet(){
	clearData();
	initData();

	m_ResultStack.reset();
	m_RecordCount=0;
	m_RecordIndex=0;
	m_OpenFlag=ofCLOSE;	
	return dbSUCCESS;
}

int TxtSet::openEx(const char* database){
	return (setDatabase((char*)database)>=0);
}
int TxtSet::executeSQL(const char* sql){
	return (doSQL((char*) sql)>=0);
}

int TxtSet::openSet(const char* Sql){
	closeSet();
	m_OpenFlag=ofOPEN;

	tFreeMem sentence;
	if (!Sql){
		if (!m_OpenTable.GetValue() ) return dbOPENFAIL;
		sentence.SetValue("select * from ");
		sentence+=m_OpenTable.GetValue();
		char* p=m_strFilter.GetValue();
		if (p && strcmp(p,"") ){
			sentence+=" where ";
			sentence+= m_strFilter.GetValue();
		}
	}else{
		sentence.SetValue((char*)Sql);
		m_OpenTable.SetValue("");
	}

	int i;
	if ((i=setRequest(sentence.GetValue()))<0) return i;
	if (Tables && Tables->Fields&& Tables->Fields->Name){
		m_IndexName.SetValue(Tables->Fields->Name);
		if (m_IndexName == "ROWID__" ) m_IndexName.SetValue("ROWID");
	}else{
		m_IndexName.Free();
	}

	if (isEOF()) return dbNOTFOUND;
	else return dbSUCCESS;
}

int TxtSet::doOpen(){
	int i;
	char* tmp=NULL;
	unsigned int size=1;
	unsigned int OutFdSepLen=strlen(OutFdSep);
	tBinData OneResult;
	tBinData SortValue;
	SortValue.Malloc(10);
	OneResult.Malloc(10);

	tStack* sort;
	for (sort=m_SortFields.Next; sort; sort=sort->Next){
		sort->Data = getMetaFieldIndex(sort->Key);
	}

	if (m_SortFields.Next) 	m_ResultStack.Type = tStack::ordINCKEY;
	else m_ResultStack.Type = tStack::ordFIFO;

	while ((i=doRequest()) == 0){
		OneResult.Clear();
		SortValue.Clear();
		if (Tables && Tables->Fields && Tables->Fields->Value){
			if (!OneResult.Cat(Tables->Fields->Value, strlen(Tables->Fields->Value))) return dbMEMORYERR;
			if (!OneResult.Cat(OutFdSep, OutFdSepLen)) return dbMEMORYERR; 
		}
		
		for (sort=m_SortFields.Next; sort; sort=sort->Next){
			tmp = getMetaField((int)sort->Data,0);
			SortValue.Cat(tmp, strlen(tmp));
		}
		if (Value && !OneResult.Cat(Value, strlen(Value))) return dbMEMORYERR;
		if (!m_ResultStack.set(0,SortValue.Data, OneResult.Data))  return dbMEMORYERR;

		m_RecordCount++;
	}
	m_Move=m_ResultStack.Next;
	if (m_OpenTable == ""){
		if (Tables && Tables->Name)
			m_OpenTable.SetValue(Tables->Name);
	}

	if (i< 0) return i;

	if (!m_Move) return dbNOTFOUND;
	return dbSUCCESS;

}

int TxtSet::requery(){
	closeSet();
	return openSet();
}

int TxtSet::setOpenTable(const char* tablename){
	if (!m_OpenTable.SetValue(tablename)) return dbMEMORYERR;
	return dbSUCCESS;
}

int TxtSet::isBOF(){
	return (m_Move == &m_ResultStack);
}

int TxtSet::isEOF(){
	if (!m_ResultStack.Next && m_OpenFlag ) doOpen();
	return (!m_Move);
}

int TxtSet::isOpen(){
	return m_OpenFlag;
}

int TxtSet::moveFirst(){
	if (!m_ResultStack.Next && m_OpenFlag ) doOpen();
	m_Move=m_ResultStack.Next;
	if (!m_Move) return dbNOTFOUND;

	return dbSUCCESS;
}

int TxtSet::moveNext(){
	if (!m_ResultStack.Next && m_OpenFlag ) doOpen();

	if (m_Move) m_Move=m_Move->Next;
	if (!m_Move) return dbNOTFOUND;
	return dbSUCCESS;
}

int TxtSet::movePrev(){
	tStack* p;
	if (!isBOF()){
		for (p=&m_ResultStack; p && p->Next != m_Move; p=p->Next);
		m_Move = p;
	}
	if (!m_Move) return dbNOTFOUND;
	return dbSUCCESS;
}

int TxtSet::getMetaFieldCount(){
	tExpression *NowField;
	int i;
	for (i=0,NowField=(tExpression *)OutFields;NowField; NowField=(tExpression *)NowField->WalkNext) if (strcmp(NowField->Name,"ROWID__")) i++;
	return i;
}

int TxtSet::getMetaFieldIndex(const char* FieldName) // index is 1,2,....
{
	tExpression *NowField;
	int i;

	for (i=1,NowField=(tExpression *)OutFields;(NowField!=NULL && strcmp(FieldName,NowField->Name) ); NowField=(tExpression *)NowField->WalkNext) if (strcmp(NowField->Name,"ROWID__")) i++;
	if (!NowField) return 0;
	return i;
}

char* TxtSet::getMetaField(int FieldIndex,int FieldType)
{
	tExpression *NowField;
	int i;
	char tmp[50];

	for (i=1,NowField=(tExpression *)OutFields;(NowField!=NULL) && (i<FieldIndex || !strcmp(NowField->Name,"ROWID__")) ; NowField=(tExpression *)NowField->WalkNext) if (strcmp(NowField->Name,"ROWID__")) i++;
	if (NowField==NULL) return NULL;
/*	if (NowField==NULL){
		sprintf(tmp,"can not find field %d.",JFieldIndex,sqlRequest.OutFields);
		return JEnv->NewStringUTF(tmp);
	}
*/
	switch (FieldType){
	case 0: // field value
		return m_GetMetaMem.SetValue(NowField->Value);
	case 1:	//table name;
		if (!NowField->Table) return NULL;
		return m_GetMetaMem.SetValue(NowField->Table->Name);
	case 2: //field name
	case 3: //field lable
		return m_GetMetaMem.SetValue(NowField->Name);
	case 4: //field type
		switch (NowField->Type){
		case dbCHAR: return m_GetMetaMem.SetValue(strCHAR);
		case dbNUMBER: return m_GetMetaMem.SetValue(strNUMBER);
		case dbDATE: return m_GetMetaMem.SetValue(strDATE);
		case dbLONG: return m_GetMetaMem.SetValue(strLONG);
		default: 
			sprintf(tmp,"unknown field:%d.",FieldIndex);
			return m_GetMetaMem.SetValue(tmp);
		}
		break;
	case 5: //field length
		sprintf(tmp,"%d",NowField->Length);
		return m_GetMetaMem.SetValue(tmp);
	case 6: //field precision
		sprintf(tmp,"%d",NowField->Precision);
		return m_GetMetaMem.SetValue(tmp);
	case 7: //field sensitivity
#ifdef CASEINSENSITIVE
		return m_GetMetaMem.SetValue("0");
#else
		return m_GetMetaMem.SetValue("1");
#endif
	case 8: //field
		return (char*)NowField;
	default: 
		break;
	}
	return NULL;
}

char* TxtSet::getValue(const char *name){
	if (isEOF() || isBOF()) return NULL;

	int index=0;
	index=getMetaFieldIndex(name);
	if (index==0) return NULL;
	
	return getValue(index);
}

char* TxtSet::getValue(int index){
	char* p=m_Move->Value;
	if (!p ) return NULL;
	int i;
	int len=strlen(OutFdSep);
	for (i=0; i<index ; i++){
		p=strstr(p, OutFdSep);
		if (!p ) return NULL;
		p+=len;
	}
	char* p1=strstr(p,OutFdSep);
	if (p1==NULL ){
		p1=strstr(p,OutRdSep);
	}

	if (p1==NULL) return NULL;
	m_GetValueMem.Malloc(p1-p);
	char* value=m_GetValueMem.GetValue();
	strncpy(value,p,p1-p);
	value[p1-p]='\0';
	
	return value;
}

long TxtSet::getLongValue(int index){
	char* p=getValue(index);
	if (!p) return 0;
	long val=0;

	tExpression *NowField = (tExpression *)getMetaField(index, 8);
	if (NowField->Type == dbDATE){
#ifdef DateStandardFormat
		DateToLong(p,DateFormat,(time_t*)&val);
#else
		sscanf(p, "%ld", &val);
#endif
	}else{
		sscanf(p, "%ld", &val);
	}

	return val;
}
long TxtSet::getLongValue(const char* name){
	if (isEOF() || isBOF()) return 0;

	int index=0;
	index=getMetaFieldIndex(name);
	if (index==0) return 0;

	return getLongValue(index);	
}
double TxtSet::getDoubleValue(int index){
	char* p=getValue(index);
	if (!p) return 0;
	double val=0;
	sscanf(p, "%lf", &val);
	return val;
}
double TxtSet::getDoubleValue(const char* name){
	if (isEOF() || isBOF()) return 0.0;

	int index=0;
	index=getMetaFieldIndex(name);
	if (index==0) return 0.0;

	return getDoubleValue(index);	
}

int TxtSet::setValue(const char *name, const char* value){
	return (m_SetValueStack.add((char*)name,(char*)value)!=NULL);
}

int TxtSet::setFilter(const char* filter){
	if (!m_strFilter.SetValue(filter) ) return dbMEMORYERR;
	return dbSUCCESS;
}

int TxtSet::setSort(const char* SortFields){
	int len=strlen(SortFields);
	char tmp[100];
	char* p, *p1;

	m_SortFields.reset();
	for (p=(char*)SortFields; p && p - SortFields < len;p=p1+1 ){
		while (*p==' ' || *p== '\t' ) p++;
		if (p-SortFields >=len) break;

		p1=strchr(p, ',');
		if (!p1) p1=(char*)SortFields + len;
		strncpy(tmp, p, p1-p);
		tmp[p1-p]='\0';
		for (int i=p1-p-1; i>0 && (tmp[i] == ' ' || tmp[i] == '\t'); i--) tmp[i]='\0';
		if (strcmp(tmp,"")==0) continue;
		if (!m_SortFields.add(tmp, tmp, true))  return dbMEMORYERR;

	}
	return dbSUCCESS;
}
int TxtSet::editRecord(){
	m_OpenFlag=ofUPDATE;
	return m_SetValueStack.reset();
}

int TxtSet::addRecord(){
	m_OpenFlag=ofINSERT;
	return m_SetValueStack.reset();
}

int TxtSet::update(){
	tFreeMem sentence;
	char tmp[100];
	int i;
	tStack* NowField;
	if (m_OpenFlag==ofUPDATE){
		if (m_IndexName==NULL || m_IndexName=="") return dbNOTSUPPORT;
		sprintf(tmp, "update %s set ",m_OpenTable.GetValue() );
		if (!sentence.SetValue(tmp)) return dbMEMORYERR;
		i=0;
		for (NowField=m_SetValueStack.Next; NowField; NowField=NowField->Next){
			if (getMetaFieldIndex(NowField->Key) <= 0) continue;
			if (i>0){
				if (!(sentence+=",")) return dbMEMORYERR;
			}
			i++;
			if (!(sentence+= DBCondition(NowField->Key, NowField->Value) )) return dbMEMORYERR;
		}
		if (!(sentence += " where ")) return dbMEMORYERR;
		if (!(sentence += DBCondition(m_IndexName.GetValue(), getValue(0)) )) return dbMEMORYERR;
		tSQLRequest updatesql;
		updatesql.setDatabase(DataBase);
		return updatesql.doSQL(sentence.GetValue());
	}else if (m_OpenFlag==ofINSERT){ 
		sprintf(tmp, "insert into %s ( ",m_OpenTable.GetValue() );
		if (!sentence.SetValue(tmp)) return dbMEMORYERR;
		tFreeMem namelist;
		tFreeMem valuelist;
		i=0;
		for (NowField=m_SetValueStack.Next; NowField; NowField=NowField->Next){
			if (getMetaFieldIndex(NowField->Key) <= 0) continue;
			if (i>0){
				if (!(namelist+=",")) return dbMEMORYERR;
				if (!(valuelist+=",")) return dbMEMORYERR;
			}
			i++;
			if (!(namelist+=NowField->Key)) return dbMEMORYERR;
			if (!(valuelist+=DBValue(NowField->Key, NowField->Value))) return dbMEMORYERR;
		}
		if (!(sentence+=namelist.GetValue())) return dbMEMORYERR;
		if (!(sentence += " ) values ( ")) return dbMEMORYERR;
		if (!(sentence+=valuelist.GetValue())) return dbMEMORYERR;
		if (!(sentence += " )" )) return dbMEMORYERR;
		tSQLRequest insertsql;
		insertsql.setDatabase(DataBase);
//		Log::add(ERRORLOGFILE,0,sentence.GetValue(),__LINE__);
		return insertsql.doSQL(sentence.GetValue());
	}else{
		return dbNOTSUPPORT;
	}
	return dbSUCCESS; 
}

int TxtSet::deleteRecord(){
	if (m_IndexName==NULL || m_IndexName=="") return dbNOTSUPPORT;
	tFreeMem sentence;
	if (!(sentence.SetValue("delete from ") )) return dbMEMORYERR;
	if (!(sentence+= m_OpenTable.GetValue() )) return dbMEMORYERR;
	if (!(sentence += " where ")) return dbMEMORYERR;
	if (!(sentence += DBCondition(m_IndexName.GetValue(), getValue(0)) )) return dbMEMORYERR;
	tSQLRequest deletesql;
	deletesql.setDatabase(DataBase);
	return deletesql.doSQL(sentence.GetValue());
}

char* TxtSet::DBCondition(const char* name, const char* value)
{
	if (!name || !value) return NULL;
	int type=dbCHAR;
	int index;
	if (strcmp(name,"ROWID")){
		if ((index=getMetaFieldIndex(name))==0) return NULL;
		if (!getMetaField(index, 4)) return NULL;
		if (m_GetMetaMem == strNUMBER) type=dbNUMBER;
		else if (m_GetMetaMem == strDATE) type= dbDATE;
	}
	tFreeMem val;
	if (!val.SetValue(value)) return NULL;
	if (!val.Replace("'","''")) return NULL;
	if (!m_DBCondition.SetValue(name)) return NULL;
	if (type==dbCHAR && strcmp(value,"")==0){
		if (!(m_DBCondition+=" is null")) return NULL;
	}else if (type == dbCHAR){
		if (!(m_DBCondition += " = '")) return NULL;
		if (!(m_DBCondition += val.GetValue() )) return NULL;
		if (!(m_DBCondition += "' ")) return NULL;
	}else if (type == dbDATE){
		char* value=val.GetValue();
		if (!tStrOp::strNcmp(value, "sysdate", 7, false) || !tStrOp::strNcmp(value, "to_date", 7, false) 
			) {
			if (!(m_DBCondition += " = ")) return NULL;
			if (!(m_DBCondition += val.GetValue() )) return NULL;
		}else{
			if (!(m_DBCondition += " = '")) return NULL;
			if (!(m_DBCondition += val.GetValue() )) return NULL;
			if (!(m_DBCondition += "' ")) return NULL;
		}
	}else {
		if (!(m_DBCondition += " = ")) return NULL;
		if (!(m_DBCondition += val.GetValue() )) return NULL;
	}
	return m_DBCondition.GetValue();
}

char* TxtSet::DBValue(const char* name, const char* value)
{
	if (!value) return NULL;
	int type=dbCHAR;
	int index;
	if (strcmp(name,"ROWID")){
		if ((index=getMetaFieldIndex(name))==0) return NULL;
		if (!getMetaField(index, 4)) return NULL;
		if (m_GetMetaMem == strNUMBER) type=dbNUMBER;
		else if (m_GetMetaMem == strDATE) type= dbDATE;
	}
	
	tFreeMem val;
	if (!val.SetValue(value)) return NULL;
	if (!val.Replace("'","''")) return NULL;

	if (type == dbCHAR){
		if (!m_DBCondition.SetValue("'")) return NULL;
		if (!(m_DBCondition += val.GetValue() )) return NULL;
		if (!(m_DBCondition += "'")) return NULL;
	}else if (type == dbDATE){
		char* value=val.GetValue();
		if (!tStrOp::strNcmp(value, "sysdate", 7, false) || !tStrOp::strNcmp(value, "to_date", 7, false) 
			){
			if (!m_DBCondition.SetValue(val.GetValue()) ) return NULL;
		}else{
			if (!m_DBCondition.SetValue("'")) return NULL;
			if (!(m_DBCondition += val.GetValue() )) return NULL;
			if (!(m_DBCondition += "'")) return NULL;
		}
	}else {
		if (!m_DBCondition.SetValue(val.GetValue()) ) return NULL;
	}
	return m_DBCondition.GetValue();
}
