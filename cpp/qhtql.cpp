#include "qhtql.h"
#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
//#include "htql.h"
#include "htqlupdate.h"
#include "referlink2.h"

#include "evalversion.h"

#undef ERRORLOG
#define ERRORLOG 0


#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



HTQLItem::HTQLItem(HTQLItem* parent){
	ItemType =0;
	NextItem = NextField = 0;
	ParentItem = parent;
	isRefNextItem= false;
}

HTQLItem::~HTQLItem(){
	reset();
}

void HTQLItem::reset(){
	HTQLTag::reset();

	HTQLItem* t=NextItem; 
	HTQLItem* t1=0;
	int isRef=isRefNextItem;
	while (!isRef && NextItem) {
		t = NextItem;
		NextItem = t->NextItem;	
		isRef=t->isRefNextItem;
		t->NextItem=0;
		delete t;
	}
	t=NextField;
	while (NextField){
		t=NextField;
		NextField=t->NextField;
		t->NextField=0;
		delete t;
	}

	ItemType =0;
	NextItem = NextField = ParentItem = 0;
	isRefNextItem=false;
}

int HTQLItem::printHtqlItem(HTQLItem* item, FILE* fw){
	HTQLItem* subitem=0;
	if (!item) return 0;
	ReferData text;
	switch (item->ItemType){
	case itemSCHEMA_REF:
		fprintf(fw, "<td> <table border=1>");
		for (subitem=item->NextItem; subitem; subitem = subitem->NextItem){
			printHtqlItem(subitem, fw);
		}
		fprintf(fw, "</table> </td>\n");
		break;
	case itemSCHEMA_DEF:
		fprintf(fw, "<tr bgcolor=lightgreen>");
		for (subitem=item->NextField; subitem; subitem = subitem->NextField){
			printHtqlItem(subitem, fw);
		}
		fprintf(fw, "</tr>\n");
		break;
	case itemSCHEMA:
		if (item->Data.P) {
			HTQLParser::toHtmlPrintable(item->Data.Seperate(), 0, &text);
			fprintf(fw, "<tr value='%s'>", text.P?text.P:"" );
		}else{
			fprintf(fw, "<tr value=''>");
		}
		for (subitem=item->NextField; subitem; subitem = subitem->NextField){
			printHtqlItem(subitem, fw);
		}
		fprintf(fw, "</tr>\n");
		break;
	case itemSET:
		fprintf(fw, "<td valign=top>");
		if (item->FieldName.P){
			HTQLParser::toHtmlPrintable(item->FieldName.Seperate(), 0, &text);
			fprintf(fw, "%s", text.P);
		}
		if (item->Data.P){
			HTQLParser::toHtmlPrintable(item->Data.Seperate(), 0, &text);
			fprintf(fw, "[%ld]%s", item->SourceOffset, text.P );
		}
		fprintf(fw, "</td>");
		break;
	}

	return 0;
}

HTQLItem* HTQLItem::copyItemData(HTQLItem* item){
	copyTag(item);
	Data.Set(item->Data.P, item->Data.L, item->Data.Type);
	FieldName.Set(item->FieldName.P, item->FieldName.L, item->FieldName.Type);
	ItemType = item->ItemType;
	isRefNextItem = false;
	return item;
}

HTQLItem* HTQLItem::getSetItem(){
	HTQLItem* item1;
	HTQLItem* item = this;
	if (!item || item->ItemType == HTQLItem::itemSET) return item;
	else if (item->ItemType == HTQLItem::itemSCHEMA){
		return (item->NextField->getSetItem());
	}else if (item->ItemType == HTQLItem::itemSCHEMA_DEF){
		return 0;
	}else if (item->ItemType == HTQLItem::itemSCHEMA_REF){
		item1=item;
		while (item1 && item1->ItemType != HTQLItem::itemSET){
			if (item1->ItemType==HTQLItem::itemSCHEMA_REF)
				item1 = item1->NextItem;
			else if (item1->ItemType==HTQLItem::itemSCHEMA_DEF) 
				item1=item1->NextItem;
			else if (item1->ItemType==HTQLItem::itemSCHEMA) 
				item1=item1->NextField;
		}
		return item1;
	}
	return 0;
}
HTQLItem* HTQLItem::getFirstSetItem(){ //return the first item if there is no Set item
	HTQLItem* item=0;
	if (ItemType==HTQLItem::itemSCHEMA && NextField) 
		item=NextField->getSetItem();
	else item=getSetItem();
	if (!item) item=this;
	return item;
}

int HTQLItem::dropField(HTQLItem** item, int index){ //index from 1;
	HTQLItem** cur_item = item;
	for (int i=1; i<index; i++){
		if (*cur_item) cur_item = &(*cur_item)->NextField;
	}
	if (*cur_item){
		HTQLItem* next_field = (*cur_item)->NextField;
		if (next_field && next_field->ItemType == itemSCHEMA_REF){
			HTQLItem* setitem = (next_field->getSetItem());
			int ref = next_field->isRefNextItem;
			if (setitem) next_field->copyItemData(setitem);
			if (!ref && next_field->NextItem){
				delete next_field->NextItem;
			}
			next_field->NextItem = 0;
		}
		if (next_field){
			next_field->NextItem = (*cur_item)->NextItem;
		}else{
			next_field = (*cur_item)->NextItem;
		}
		(*cur_item)->NextItem = 0;
		(*cur_item)->NextField=0;
		delete (*cur_item);
		*cur_item = next_field;
		return 0;
	}
	return 1;
}

int HTQLItem::mergeField(HTQLItem** item, int index){ //index from 1;
	HTQLItem** cur_item = item;
	for (int i=1; i<index; i++){
		if (*cur_item) cur_item = &(*cur_item)->NextField;
	}
	if (*cur_item){
		HTQLItem* next_field = (*cur_item)->NextField;
		if (next_field){
			HTQLItem* setitem = (next_field->getSetItem());
			if (setitem){
				(*cur_item)->Data.Cat(setitem->Data.P, setitem->Data.L);
				(*cur_item)->E.Set(setitem->E.P, setitem->E.L, setitem->E.Type);
			}
			(*cur_item)->NextField = next_field->NextField;
			next_field->NextField = 0;
			delete next_field;
		}
		return 0;
	}
	return 1;
}

int HTQLItem::insertField(HTQLItem** item, int index, HTQLItem* parent){ //index from 1;
	HTQLItem** cur_item = item;
	for (int i=1; i<index; i++){
		if (!*cur_item) (*cur_item) = new HTQLItem(parent);
		cur_item = &(*cur_item)->NextField;
	}
	if (!*cur_item) {
		(*cur_item) = new HTQLItem(parent);
	}else{
		HTQLItem* next_field = (*cur_item);
		(*cur_item) = new HTQLItem(parent);
		(*cur_item)->NextField = next_field;
		if (index==1 && next_field){
			(*cur_item)->NextItem = next_field->NextItem;
			(*cur_item)->isRefNextItem = next_field->isRefNextItem;
			next_field->NextItem = 0;
		}
	}
	return 0;
}

int HTQLItem::setFieldData(HTQLItem** item, int index, char*p, long len, int copy, HTQLItem* parent){ //index from 1;
	HTQLItem** cur_item = item;
	for (int i=1; i<index; i++){
		if (!*cur_item) (*cur_item) = new HTQLItem(parent);
		cur_item = &(*cur_item)->NextField;
	}
	if (!*cur_item) (*cur_item) = new HTQLItem(parent);
	(*cur_item)->Data.Set(p, len, copy);

	return 0;
}

int HTQLItem::setFieldName(HTQLItem** item, int index, char*name, HTQLItem* parent){ //index from 1;
	HTQLItem** cur_item = item;
	for (int i=1; i<index; i++){
		if (!*cur_item) (*cur_item) = new HTQLItem(parent);
		cur_item = &(*cur_item)->NextField;
	}
	if (!*cur_item) (*cur_item) = new HTQLItem(parent);
	(*cur_item)->FieldName.Set(name, strlen(name), true);

	return 0;
}

HTQLTag::HTQLTag(){
	TagType = 0;
	SourceOffset = 0;
}

HTQLTag::~HTQLTag(){
	reset();
}

void HTQLTag::reset(){
	TagType = 0;
	SourceOffset = 0;
	S.reset();
	E.reset();
}

HTQLTag* HTQLTag::copyTag(HTQLTag* From, int newMem){
	TagType=From->TagType;
	SourceOffset = From->SourceOffset;
	S.Set(From->S.P, From->S.L, From->S.P?newMem:false);
	E.Set(From->E.P, From->E.L, From->E.P?newMem:false);
	return this;
}

HTQLScope::HTQLScope(){
	SerialNo = 0;
	NextTag = 0;
	PreviousTag=0;
	EnclosedTag=0;
}

HTQLScope::~HTQLScope(){
	reset();
}

void HTQLScope::reset(){
	HTQLTag::reset();

	HTQLScope* t=NextTag; 
	HTQLScope* t1=0;
	while (NextTag) {
		t = NextTag;
		NextTag = t->NextTag;
		t->NextTag=0;
		delete t;
	}
	SerialNo = 0;
	PreviousTag=0;
	EnclosedTag=0;
}

TagOptions::TagOptions(){
	Sen=false;
	PlainIncl=false;
	HyperIncl=true;
	Iran=false;
	Recur = true;
	Sep=false;
	NoEmbedding=false;
	WildMatching=matNULL;
}

TagOptions::~TagOptions(){
	reset();
}
void TagOptions::reset(){
	Sen=false;
	PlainIncl=false;
	HyperIncl=true;
	Iran=false;
	Recur = true;
	Sep=false;
	NoEmbedding=false;
	NoIFrame=false;
	WildMatching=matNULL;
}

void TagOptions::operator = (TagOptions& option){
	Sen = option.Sen;
	PlainIncl = option.PlainIncl;
	HyperIncl = option.HyperIncl;
	Iran = option.Iran;
	Recur = option.Recur;
	Sep = option.Sep;
	NoEmbedding = option.NoEmbedding;
	NoIFrame=option.NoIFrame;
	WildMatching=option.WildMatching; 
}

int tRange::NONE=-10000;

tRange::tRange(){
	From=To=NONE;
	Per = 0;
}
tRange::~tRange(){
}

HTQLFunction::HTQLFunction(){
	FunPrepare=0;
	FunComplete=0;
	FunItem=0;
	FunItemSetPrepare=0;
	FunItemSetComplete=0;
	FunSchemPrepare=0;
	FunSchemComplete=0;
	FunData=0;
	FunFinalRelease=0;
}

HTQLFunction::~HTQLFunction(){
	reset();
}

void HTQLFunction::reset(){
	FunPrepare=0;
	FunComplete=0;
	FunItem=0;
	FunItemSetPrepare=0;
	FunItemSetComplete=0;
	FunSchemPrepare=0;
	FunSchemComplete=0;
	FunData=0;
	FunFinalRelease=0;
}

HTQLTagSelection::HTQLTagSelection(){
	Results = 0;
	ParentItem=0;
	Vars = 0;
	Operations = 0;
	TagScope = 0;
	CurrentTagScope = 0;
	SourceOffset = 0;
	ReferVars=0;
	IsReversedTag = false;
	Indx=0; 
	IndxMax=IndxNum=0;
}

HTQLTagSelection::~HTQLTagSelection(){
	reset();
}
void HTQLTagSelection::reset(){
	if (Vars){
		delete Vars;
		Vars = 0;
	}
	if (TagScope) {
		delete TagScope;
		TagScope = 0;
	}
	if (Results){
		delete Results;
		Results = 0;
	}
	if (Operations){
		ReferLink* p=Operations;
		while (p){
			if (p->Data == opCONDITION){// is expression at value
				delete (tExprCalc*)p->Value.P;
			}
			p=p->Next;
		}
		delete Operations;
		Operations=0;
	}
	ParentItem=0;
	CurrentTagScope=0;
	SourceOffset = 0;
	Data.reset();
	Sentence.reset();
	Syntax.reset();
	Tag.reset();
	if (Indx) {
		delete [] Indx; 
		Indx=0; 
	}
	IndxMax=IndxNum=0;
	Options.reset();
	LastTag.reset();
	ReferVars=0;
	IsReversedTag = false; 
}
/*
int HTQLTagSelection::parse(){
	int i=parseSentence();
	if (i) return i;
	i=parseData();
	if (i) return i;
	return 0;
}
*/
int HTQLTagSelection::setIndxValue(int index0, int from, int to, int per){
	if (index0>=IndxMax){
		tRange* range1=new tRange[IndxMax+10]; 
		if (Indx) {
			memcpy(range1, Indx, sizeof(tRange)*(IndxMax)); 
			delete [] Indx; 
		}
		IndxMax+=10; 
		Indx=range1; 
	}
	Indx[index0].From=from; 
	Indx[index0].To=to; 
	Indx[index0].Per=per;
	if (IndxNum<=index0) IndxNum=index0+1; 
	return 0;
}

int HTQLTagSelection::parseSentence(){
	int i=0, from=0;
	Syntax.setSentence(Sentence.P, &Sentence.L, false);
	Tag.copyTag(&LastTag, true);

	if (Syntax.Type == QLSyntax::synQL_WORD){
		parseAttributes();
		if (Syntax.Type == QLSyntax::synQL_COLON) {
			Syntax.match();
			return parseSentence();
		}else 
			return 0;
	}

	if (Syntax.Type == QLSyntax::synQL_SLASH || Syntax.Type == QLSyntax::synQL_LTAG ){
		if (Syntax.Type == QLSyntax::synQL_SLASH ){
			parsePlainTag();
		}else if (Syntax.Type == QLSyntax::synQL_LTAG){
			parseHyperTag();
			i=parseFilters();
			if (i) return i;
		}
		i=parseFilters();
		if (i) return i;
		if (Tag.TagType == HTQLTag::tagPLAIN && Syntax.Type!= QLSyntax::synQL_SLASH) 
			return htqlSYNTAXERR;
		else if (Tag.TagType == HTQLTag::tagHTML && Syntax.Type!= QLSyntax::synQL_RTAG) 
			return htqlSYNTAXERR;
		else if (Tag.TagType == HTQLTag::tagXML && Syntax.Type!= QLSyntax::synQL_RTAG) 
			return htqlSYNTAXERR;

		Syntax.match();
	}

	i=0;
	while (Syntax.Type == QLSyntax::synQL_NUMBER || Syntax.Type == HTQLTagSelSyntax::synQL_PER || Syntax.Type == QLSyntax::synQL_LBRACE){
		if (Syntax.Type == QLSyntax::synQL_NUMBER || Syntax.Type == QLSyntax::synQL_LBRACE){
			parseIndxRange1(i, 0); 
			if (Syntax.Type == QLSyntax::synQL_DASH){
				Syntax.match();
				parseIndxRange1(i, 1); 
			}
		}

		if (Syntax.Type == HTQLTagSelSyntax::synQL_PER){
			Syntax.match();
			parseIndxRange1(i, 2); 
		}
		i++;
		if (Syntax.Type != QLSyntax::synQL_COMMA) break;
		Syntax.match();
	}

	if (Syntax.Type == QLSyntax::synQL_COLON){
		Syntax.match();
		parseAttributes();
		if (!Tag.S.P){
			Tag.TagType=HTQLTag::tagHTML;
		}
	}
	return 0;
}
int HTQLTagSelection::parseIndxRange1(int index0, int from0_to1_per2){
	int err=0; 

	int from=0;
	if (Syntax.Type == QLSyntax::synQL_NUMBER){
		from=atoi(Syntax.Sentence+Syntax.Start); 
		Syntax.match();
	}else if (Syntax.Type == QLSyntax::synQL_LBRACE){
		tExprCalc expr; 
		expr.setExpression(Syntax.Sentence+Syntax.Start);
		err=expr.parse(synEXP_LEFTBRACE); 
		if (err==0) err=expr.calculate();
		from=(int) expr.getLong(); 
		Syntax.Next = Syntax.Start+expr.ExprSentence->Start;
		Syntax.NextLen = 0;
		Syntax.match();
		Syntax.match();
	}else{
		return -1; 
	}
	if (from0_to1_per2==0) setIndxValue(index0, from, from, 1); 
	else{
		if (!Indx) setIndxValue(index0, 1, 0, 1);
		if (from0_to1_per2==1) Indx[index0].To=from; 
		else Indx[index0].Per=from; 
	}
	return err; 
}

int HTQLTagSelection::getOption(char* name, int len){
	//enum{optUNKNOW, optSEN=1, optINSEN, optINCL, optEXCL, optIRAN, optORAN, optSEP };
	if ((len==3)&&!tStrOp::strNcmp(name,"SEN",len, false)) return optSEN;
	if ((len==5)&&!tStrOp::strNcmp(name,"INSEN",len, false)) return optINSEN;
	if ((len==4)&&!tStrOp::strNcmp(name,"INCL",len, false)) return optINCL;
	if ((len==4)&&!tStrOp::strNcmp(name,"EXCL",len, false)) return optEXCL;
	if ((len==3)&&!tStrOp::strNcmp(name,"SEP",len, false)) return optSEP;
	if ((len==5)&&!tStrOp::strNcmp(name,"RECUR",len, false)) return optRECUR;
	if ((len==7)&&!tStrOp::strNcmp(name,"NORECUR",len, false)) return optNORECUR;
	if ((len==4)&&!tStrOp::strNcmp(name,"IRAN",len, false)) return optIRAN;
	if ((len==4)&&!tStrOp::strNcmp(name,"ORAN",len, false)) return optORAN;
	if ((len==5)&&!tStrOp::strNcmp(name,"EMBED",len, false)) return optEMBED;
	if ((len==7)&&!tStrOp::strNcmp(name,"NOEMBED",len, false)) return optNOEMBED;
	if ((len==6)&&!tStrOp::strNcmp(name,"IFRAME",len, false)) return optIFRAME;
	if ((len==8)&&!tStrOp::strNcmp(name,"NOIFRAME",len, false)) return optNOIFRAME;
	if ((len==5)&&!tStrOp::strNcmp(name,"MATCH",len, false)) return optMATCH;

	return optUNKNOW;
}

int HTQLTagSelection::parseFilters(){
	while (Syntax.Type != QLSyntax::synQL_RTAG && 
		Syntax.Type != QLSyntax::synQL_SLASH &&
		Syntax.Type != QLSyntax::synQL_END &&
		Syntax.Type != QLSyntax::synQL_UNKNOW
		){
		if (Syntax.Type == QLSyntax::synQL_WORD){
			int opt=getOption(Syntax.Sentence + Syntax.Start, Syntax.StartLen);
			if (opt== optUNKNOW ) return htqlSYNTAXERR;
			switch (opt){
			case optSEN: 
				Options.Sen = true;
				break;
			case optINSEN: 
				Options.Sen = false;
				break;
			case optINCL: 
				if (Tag.TagType == HTQLTag::tagPLAIN)
					Options.PlainIncl = true;
				else 
					Options.HyperIncl = true;
				break;
			case optEXCL: 
				if (Tag.TagType == HTQLTag::tagPLAIN)
					Options.PlainIncl = false;
				else 
					Options.HyperIncl = false;
				break;
			case optIRAN: 
				Options.Iran = true;
				break;
			case optORAN: 
				Options.Iran = false;
				break;
			case optRECUR: 
				Options.Recur = true;
				break;
			case optNORECUR: 
				Options.Recur = false;
				break;
			case optSEP: 
				Options.Sep = true;
				break;
			case optEMBED: 
				Options.NoEmbedding = false;
				break;
			case optNOEMBED: 
				Options.NoEmbedding = true;
				break;
			case optIFRAME: 
				Options.NoIFrame = false;
				break;
			case optNOIFRAME: 
				Options.NoIFrame = true;
				break;
			case optMATCH: 
				Options.WildMatching = Options.matMATCH;
				break;
			default:
				break;
			}
			Syntax.match();
		}else if (Syntax.Type == QLSyntax::synQL_STRING){
			//it is tag-mapping enclosed by ' or ": word '=~' string 
			ReferLink* r=new ReferLink;
			r->Name.Set(Syntax.Sentence + Syntax.Start, Syntax.StartLen, true);
			r->Data = opTRANSFORM;
			ReferLink** t=&Operations;
			while (*t) t=&((*t)->Next);
			(*t) = r;
			Syntax.match();
		}else if (Syntax.Type == QLSyntax::synQL_LBRACE){
			// it is tag-filtering: enclosed by '(' and ')'
			ReferLink* r, *p;
			tExprCalc* expr = new tExprCalc;
			expr->setExpression(Syntax.Sentence + Syntax.Start, 0);
			expr->parse();
			for (tExprField* field = expr->Context->Fields; field; field=field->WalkNext){
				for (p=Vars; p && p->Name.Cmp(field->Name, strlen(field->Name),Options.Sen)
					; p=p->Next);
				if (!p){
					p=new ReferLink;
					p->Name.Set(field->Name, strlen(field->Name),true);
					p->Next = Vars;
					Vars=p;
				}
			}

			r=new ReferLink;
			r->Name.Set(expr->ExprSentence->Sentence, expr->ExprSentence->Start, true);
			r->Data = opCONDITION;
			r->Value.Set((char*) expr, 0);
			ReferLink** op=&Operations;
			while (*op) op=&((*op)->Next);
			(*op) = r;
			
			Syntax.Next = Syntax.Start+expr->ExprSentence->Start;
			Syntax.NextLen = 0;
			Syntax.match();
			Syntax.match();
			if (Syntax.Type == QLSyntax::synQL_RBRACE) Syntax.match();
		}else{
			// or return htqlSYNTAXERR; ???
			Syntax.match();
		}
	}
	return 0;
}

int HTQLTagSelection::parsePlainTag(){
	if (!Syntax.match(QLSyntax::synQL_SLASH)) return htqlSYNTAXERR;
	Tag.TagType = HTQLTag::tagPLAIN;
	//Options.PlainIncl = false;

	if (Syntax.Type == QLSyntax::synQL_DASH){
		IsReversedTag = true;
		Syntax.match();
	}

	int has_tag=true; 
	Options.WildMatching = 0;
	if (Syntax.Type == QLSyntax::synQL_STAR || Syntax.Type == QLSyntax::synQL_AT){
		if (Syntax.Type == QLSyntax::synQL_STAR)
			Options.WildMatching = Options.matSTAR;
		else
			Options.WildMatching = Options.matAT;

		if (Syntax.Sentence[Syntax.Start+Syntax.StartLen]=='/' || tStrOp::isSpace(Syntax.Sentence[Syntax.Start+Syntax.StartLen]))
			has_tag=false; 
		Tag.S.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
		Syntax.match();
	}

	if (has_tag){
		if (Syntax.Type == QLSyntax::synQL_STRING){
			Tag.S.Set(Syntax.Sentence+Syntax.Start+1, Syntax.StartLen-2, true);
		}else if (Syntax.Type == QLSyntax::synQL_WORD){
			Tag.S.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
		}else {
			Tag.S.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
//			return htqlSYNTAXERR;
		}
	}
	Syntax.match();
	parseSpecial(&Tag.S);

	if (Syntax.Type != QLSyntax::synQL_TILE) {
		Options.Sep = true;
		return SUCCESS;
	}
	Syntax.match();

	if (Syntax.Type == QLSyntax::synQL_STRING){
		Tag.E.Set(Syntax.Sentence+Syntax.Start+1, Syntax.StartLen-2, true);
//	}else if (Syntax.Type == QLSyntax::synQL_WORD){  //is attributes
//		Tag.E.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
//	}else {
//		return htqlSYNTAXERR;
		Syntax.match();
		parseSpecial(&Tag.E);
	}

	Options.Sep = false;
	return SUCCESS;
}

int HTQLTagSelection::parseHyperTag(){
	if (!Syntax.match(QLSyntax::synQL_LTAG)) return htqlSYNTAXERR;
	Tag.TagType = HTQLTag::tagHTML;
	//Options.HyperIncl = true;

	if (Syntax.Type == QLSyntax::synQL_DASH){
		IsReversedTag = true;
		Syntax.match();
	}
	int has_tag=true; 
	Options.WildMatching = Options.matNULL;
	if (Syntax.Type == QLSyntax::synQL_STAR || Syntax.Type == QLSyntax::synQL_AT){
		if (Syntax.Type == QLSyntax::synQL_STAR)
			Options.WildMatching = Options.matSTAR;
		else
			Options.WildMatching = Options.matAT;

		if (Syntax.Sentence[Syntax.Start+Syntax.StartLen]=='>' || tStrOp::isSpace(Syntax.Sentence[Syntax.Start+Syntax.StartLen]))
			has_tag=false; 
		Tag.S.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
		Syntax.match();
	}

	if (has_tag){
		if (Syntax.Type == QLSyntax::synQL_STRING){
			Tag.S.Set(Syntax.Sentence+Syntax.Start+1, Syntax.StartLen-2, true);
			Syntax.match();
		}else if (Syntax.Type == QLSyntax::synQL_WORD){
			Tag.S.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
			Syntax.match();
		}else {
			return htqlSYNTAXERR;
		}
	}
	Options.Sep = false;
	return SUCCESS;
}

int HTQLTagSelection::parseSpecial(ReferData* d){
	d->Seperate();
	size_t len;
	size_t i,j;
	for (len=0; d->P[len]; len++){
		if (d->P[len] == '\\'){
			i=1;
			if (d->P[len+1] == 't'){
				d->P[len]='\t';
			}else if (d->P[len+1] == 'n'){
				d->P[len]='\n';
			}else if (d->P[len+1] == 'r'){
				d->P[len]='\r';
			}else if (isdigit(d->P[len+1]) ){
				int t=0;
				for (i=1; i<4; i++){
					if (!isdigit(d->P[len+i]) )
						break;
					t = t*8 + d->P[len+i] - '0';
				}
				d->P[len]=t;
			}else{
				d->P[len]=d->P[len+1];
			}
			for (j=i+1; j<=d->L-len; j++){
				d->P[len+j-i] = d->P[len+j];
			}
			d->L-=i;
		}
	}
	return 0;
}

int HTQLTagSelection::parseAttributes(){
	while (Syntax.Type == QLSyntax::synQL_WORD){
		// set variables
		ReferData name;
		name.Set(Syntax.Sentence + Syntax.Start, Syntax.StartLen, true);
		tStrOp::replaceInplace(name.P, ":","_");
		tStrOp::replaceInplace(name.P, "-","_");
		
		ReferLink * p;
		for (p=Vars; p && 
			p->Name.Cmp(name.P, name.L, Options.Sen)
			; p=p->Next);
		if (!p){
			p= new ReferLink;
			p->Name.Set(name.P, name.L, true);
			p->Next = Vars;
			Vars = p;
		}
		// set output fileds
		if (!Results){
			Results = new HTQLItem(ParentItem);
		}
		Results->ItemType = HTQLItem::itemSCHEMA_DEF;
		HTQLItem* m = Results;
		while (m->NextField) m=m->NextField;
		m->NextField = new HTQLItem(ParentItem);
		m=m->NextField;
		m->FieldName.Set(p->Name.P, p->Name.L, true);

		// go to the next attribute
		Syntax.match();
		if (Syntax.Type != QLSyntax::synQL_COMMA) break;
		Syntax.match();
	}
	return SUCCESS;
}
int HTQLTagSelection::checkIndxRange(long index, int& inRange, int& inGroup, long total_tuple){
	inRange=true;
	inGroup=false;
	int from, to; 
	if (IndxNum){
		inRange=false;
		for (int i=0; i<IndxNum; i++){
			if (Indx[i].Per){
				int inThis=true;

				from=Indx[i].From; to=Indx[i].To;
				if (from<=0 && from>tRange::NONE) {
					from=total_tuple+from; 
					if (from<0) from=0; 
				}
				if (to<=0 && to>tRange::NONE) {
					to=total_tuple+to; 
					if (to<from) to=-1; 
				}

				if (from > index) inThis = false;
				if (index > to) inThis = false;
				if ((index-from) % Indx[i].Per ) {
					inThis = false;
					inGroup=true;
				}
				if (inThis) {
					inRange = true;
					break;
				}
			}else{
				break;
			}
		}
	}
	return inRange || inGroup;
}

int HTQLTagSelection::parseData(){
	int is_first_tag=false;
	if (!Tag.S.P){
		if (LastTag.S.P) Tag.copyTag(&LastTag, true);
	}
	parseTagScope();
	CurrentTagScope = TagScope;
	movetoNextTag();

	//check if I need to count total
	long total_tuple=0; 
	int need_total=false; 
	for (int i=0; i<IndxNum; i++){
		if ((Indx[i].To<=0 && Indx[i].To!=tRange::NONE)|| (Indx[i].From<=0 && Indx[i].From!=tRange::NONE)) {
			need_total=true; 
			break; 
		}
	}
	//count total tuples
	if (need_total){
		total_tuple=0; 
		HTQLScope* keep=CurrentTagScope; 
		while (CurrentTagScope){
			total_tuple++;
			movetoNextTag();
		}
		CurrentTagScope=keep;
	}

	long index=0;
	HTQLItem** res=&Results;
	HTQLItem* field=0, *newfield=0;
	while (*res) res=&(*res)->NextItem;
	HTQLItem* GroupItem=0;
	while (CurrentTagScope){
		index++;
		int inGroup=false;
		int inRange=true;
		if (Options.Iran){
			checkIndxRange(index, inRange, inGroup, total_tuple);
		}
		//till now it is in range;
		int isResult=true;
		ReferLink* var;
		if (inRange || inGroup) {
			setVariableValues(CurrentTagScope);
			tExprCalc* expr=0;
			for (ReferLink* op=Operations; op; op=op->Next){
				if (op->Data == opCONDITION){
					expr = (tExprCalc*) op->Value.P;
					for (var=Vars; var; var=var->Next){
						expr->setField(var->Name.P, var->Value.P);
					}
					expr->calculate();
					if (!expr->getBoolean()){
						isResult=false;
						break;
					}
				}else if (op->Data == opTRANSFORM){
				}
			}
			//till now it also satisfies condition
		}

		//set schema data
		if (inRange || inGroup){
			if (isResult && !inGroup){
				(*res) = new HTQLItem(ParentItem);
				(*res)->ItemType = HTQLItem::itemSET;
				setTagVal(CurrentTagScope, *res, 
					(Tag.TagType == HTQLTag::tagPLAIN && Options.PlainIncl) ||
					(Tag.TagType != HTQLTag::tagPLAIN && Options.HyperIncl)
					);
				if ( Results->ItemType == HTQLItem::itemSCHEMA_DEF){
					(*res)->ItemType = HTQLItem::itemSCHEMA;
					newfield=*res;
					for (field = Results->NextField; field; field=field->NextField){
						newfield->NextField = new HTQLItem(*res);
						newfield = newfield->NextField;
						for (var=Vars; var; var=var->Next){
							if (!var->Name.Cmp(field->FieldName.P, field->FieldName.L,Options.Sen))
								break;
						}
						
						if (var){
							newfield->Data.Set(var->Value.P, var->Value.L, true);
							newfield->SourceOffset = var->Data;
						}else{
							newfield->Data.Set("", 0, true);
							newfield->SourceOffset = 0;
						}
						newfield->S.Set(newfield->Data.P, 0, false);
						newfield->E.Set(newfield->Data.P+newfield->Data.L, 0, false);
						newfield->TagType = HTQLTag::tagPLAIN;
					}
				}
				GroupItem=*res;
				res = & (*res)->NextItem;
			}else if (isResult && GroupItem && GroupItem->ItemType == HTQLItem::itemSET 
				&& Results && Results->ItemType == HTQLItem::itemSET && !Options.Sep){
				HTQLItem newitem(0);
				setTagVal(CurrentTagScope, &newitem, 
					(Tag.TagType == HTQLTag::tagPLAIN && Options.PlainIncl) ||
					(Tag.TagType != HTQLTag::tagPLAIN && Options.HyperIncl)
					);
				GroupItem->Data.Cat(newitem.Data.P, newitem.Data.L);
			}
		}

		movetoNextTag();
	}

	if (Options.Sep){
		sepResultTags();
	}

	//check outer range
	if (!Options.Iran){
		res=&Results;
		while ((*res) && (*res)->ItemType==HTQLItem::itemSCHEMA_DEF) res=&(*res)->NextItem;


		//count total tuples
		if (need_total){
			total_tuple=0; 
			HTQLItem* keep=*res; 
			while (keep){
				total_tuple++;
				keep=keep->NextItem;
				while (keep && keep->ItemType==HTQLItem::itemSCHEMA_DEF) keep=keep->NextItem;
			}
		}

		index=0;
		while (*res){
			index++;
			int inGroup=false;
			int inRange=true;
			checkIndxRange(index, inRange, inGroup, total_tuple);
			if (inRange){
				res=&(*res)->NextItem;
			}else if (inGroup){ //add to group item and delete the group item, no grouping is supported now
				//add to group item
				//...

				//delete current item
				HTQLItem* item=*res;
				(*res)=item->NextItem;
				item->NextItem=0;
				delete item; //safe???
			}else{
				HTQLItem* item=*res;
				(*res)=item->NextItem;
				item->NextItem=0;
				delete item; //safe???
			}
			while ((*res) && (*res)->ItemType==HTQLItem::itemSCHEMA_DEF) res=&(*res)->NextItem;
		}
	}

	if (IsReversedTag) {
		reverseResultTags();
	}


	return 0;
}

int HTQLTagSelection::setTagVal(HTQLScope* CurrTag, HTQLItem* res, int Incl){
	//Incl<0, force excl
	res->copyTag(CurrTag, false);
	if (Incl>0 || (Options.Sep && Incl==0)){
		if (CurrTag->E.P){
			res->Data.Set(CurrTag->S.P, CurrTag->E.P - CurrTag->S.P + CurrTag->E.L, false);
		}else if (CurrTag->EnclosedTag && !Options.Sep){
			if (CurrTag->SourceOffset<CurrTag->EnclosedTag->SourceOffset){
				res->Data.Set(CurrTag->S.P, CurrTag->EnclosedTag->S.P-CurrTag->S.P, false);
			}else if (CurrTag->TagType==HTQLTagDataSyntax::synQL_DATA) {
				res->Data.Set(CurrTag->S.P, CurrTag->E.P-CurrTag->S.P, false);
			}else {
				res->Data.Set(CurrTag->S.P, CurrTag->S.L, false);
			}
		}else if (Options.Sep) {
			res->Data.Set(CurrTag->S.P, CurrTag->S.L, false);
		}else{
			//(*res)->Data.Set(CurrTag->S.P, Data.P+Data.L-CurrTag->S.P-CurrTag->S.L, false);
			res->Data.Set(CurrTag->S.P, Data.P+Data.L-CurrTag->S.P, false);
		}
		if (res->Data.L<0) res->Data.L = 0; 
		res->SourceOffset = CurrTag->SourceOffset;
	}else{
		if (CurrTag->E.P){
			res->Data.Set(CurrTag->S.P+CurrTag->S.L, CurrTag->E.P - CurrTag->S.P - CurrTag->S.L, false);
		}else if (CurrTag->EnclosedTag&& !Options.Sep){
			if (CurrTag->SourceOffset<CurrTag->EnclosedTag->SourceOffset){
				res->Data.Set(CurrTag->S.P+CurrTag->S.L, CurrTag->EnclosedTag->S.P-CurrTag->S.P-CurrTag->S.L, false);
			}else if (CurrTag->TagType==HTQLTagDataSyntax::synQL_DATA) {
				res->Data.Set(CurrTag->S.P, CurrTag->E.P-CurrTag->S.P, false);
			}else {
				res->Data.Set(CurrTag->S.P, CurrTag->S.L, false);
			}
		}else{
			res->Data.Set(CurrTag->S.P+CurrTag->S.L, Data.P+Data.L-CurrTag->S.P-CurrTag->S.L, false);
		}
		if (res->Data.L<0) res->Data.L = 0; 
		res->SourceOffset = CurrTag->SourceOffset + CurrTag->S.L;

		//if (IsXML && CurrTag->NextTag && 
	
		char* tx=res->Data.P; 
		while (tx && tx[0] && tStrOp::isSpace(tx[0])) tx++; 
		if (tx && !tStrOp::strNcmp(tx, "<![CDATA[", 9, false)){
			tx+=9; 
			char* tx1=strstr(tx, "]]>"); 
			if (!tx1) {
				tx1=res->Data.P+res->Data.L;
			}
			res->SourceOffset+=tx-res->Data.P; 
			res->Data.P=tx; 
			res->Data.L=tx1-tx; 
		}

		res->S.Set(res->Data.P,0, false);
		res->E.Set(res->Data.P+res->Data.L,0, false);
	}
	return 0;
}
int HTQLTagSelection::sepResultTags(){
	//for <tag SEP >
	HTQLItem** res = &Results;
	while ((*res) && (*res)->ItemType==HTQLItem::itemSCHEMA_DEF) res=&(*res)->NextItem;

	long offset=0, len=0, len1;
	int incl_tag=true;
	if (Tag.TagType == HTQLTag::tagHTML|| Tag.TagType == HTQLTag::tagXML){
		incl_tag=Options.HyperIncl;
	}else if (Tag.TagType == HTQLTag::tagPLAIN){
		incl_tag=Options.PlainIncl;
	}

	//deal with the first item
	HTQLItem* first_item=0;
	HTQLItem* item=0, *next_item=0;
	if (*res) item=(*res)->getFirstSetItem();
	if (!(*res) || item->SourceOffset>=SourceOffset ){
		len=item?item->SourceOffset-SourceOffset:Data.L;

		first_item=new HTQLItem(Results?Results->ParentItem:0);
		first_item->Data.Set(Data.P, len, false);
		first_item->S.Set(Data.P, 0, false);
		first_item->E.Set(Data.P+len, 0, false);
		//first_item->TagType = Results->TagType;
		first_item->SourceOffset = SourceOffset;

		first_item->NextItem=(*res);
		(*res)=first_item;
		res=&first_item->NextItem;
	}
	while (*res){
		//get the current item
		item=(*res)->getFirstSetItem();

		offset = item->SourceOffset-SourceOffset;//relative offset
		len=item->Data.L;

		if ((*res)->NextItem){
			//get the next item
			next_item=(*res)->NextItem->getFirstSetItem();

			len1=next_item->SourceOffset-item->SourceOffset;
			if (offset+len1>Data.L) len1=Data.L-offset;
		}else{
			len1=Data.L-offset;
		}

		if (incl_tag) {
			item->Data.Set(Data.P+offset, len1, false);
			item->SourceOffset = SourceOffset + offset;
		}else{
			if (len1>=len){
				item->Data.Set(Data.P+offset+len, len1-len, false);
				item->SourceOffset = SourceOffset + offset+len;
			}
		}

		res = &(*res)->NextItem;
		while ((*res) && (*res)->ItemType==HTQLItem::itemSCHEMA_DEF) res=&(*res)->NextItem;
	}
	return 0;
}

int HTQLTagSelection::reverseResultTags(){
	//for <-tag>
	HTQLItem** res = &Results;
	while ((*res) && (*res)->ItemType==HTQLItem::itemSCHEMA_DEF) res=&(*res)->NextItem;
	long offset=0, offset1, len, len1;
	HTQLItem* item=0;
	while (*res){
		item=(*res)->getFirstSetItem();

		offset1 = item->SourceOffset-SourceOffset;
		len1=item->Data.L;

		len = offset1 - offset;
		if (len>=0 && offset <= Data.L){
			if (offset+len < Data.L){
				item->Data.Set(Data.P+offset, len, false);
			}else{
				item->Data.Set(Data.P+offset, Data.L-offset, false);
			}
			item->SourceOffset = SourceOffset + offset;
		}else{
			item->Data.Set(Data.P+offset, 0, false);
			item->SourceOffset = SourceOffset + offset;
		}
		
		if (offset1+len1>offset) offset = offset1+len1;

		res = &(*res)->NextItem;
		while ((*res) && (*res)->ItemType==HTQLItem::itemSCHEMA_DEF) res=&(*res)->NextItem;
	}
	if (offset < Data.L){
		(*res) = new HTQLItem(Results?Results->ParentItem:0); //last item
		(*res)->Data.Set(Data.P+offset, Data.L-offset, false);
		(*res)->S.Set(Data.P+offset, 0, false);
		(*res)->E.Set(Data.P+Data.L, 0, false);
		//(*res)->TagType = Results->TagType;
		(*res)->SourceOffset = SourceOffset + offset;
	}

	//merging all items
	if (Results){
		HTQLItem* merged_result=new HTQLItem(Results->ParentItem); 
		merged_result->SourceOffset=Results->SourceOffset; 
		res = &Results;
		while (*res){
			item=(*res)->getFirstSetItem();

			merged_result->Data.Cat(item->Data.P, item->Data.L); 

			res = &(*res)->NextItem;
			while ((*res) && (*res)->ItemType==HTQLItem::itemSCHEMA_DEF) res=&(*res)->NextItem;
		}
		merged_result->S.Set(merged_result->Data.P, 0, false); 
		merged_result->E.Set(merged_result->Data.P+merged_result->Data.L, 0, false);

		delete Results; 
		Results=merged_result;
	}

	return 0;
}

int HTQLTagSelection::dumpHTMLScope(HTQLScope* start, const char* filename){
	FILE* f=fopen(filename, FILE_WRITE);
	if (!f) return -1;
	ReferData str1, str2, str3;
	for (HTQLScope* t=start; t; t=t->NextTag){
		if (t->NextTag){
			str1.Set(t->S.P, t->NextTag->S.P-t->S.P, true);
		}else{
			str1.Set(t->S.P, t->S.L, true);
		}
		tStrOp::replaceInplace(str1.P, "\n","");
		tStrOp::replaceInplace(str1.P, "\r","");
		str2.Set(t->E.P, t->E.L, true);
		if (t->EnclosedTag){
			str3.Set(t->EnclosedTag->S.P, t->EnclosedTag->S.L, true);
		}else{
			str3.Set("", 0, true);
		}
		fprintf(f, "%6ld:->%-6ld ## %6ld:->%-6ld *** %s *** %s @@@ %s\n", t->SerialNo, t->EnclosedTag?t->EnclosedTag->SerialNo:0, t->SourceOffset, t->EnclosedTag?t->EnclosedTag->SourceOffset:0, str1.P, str2.P, str3.P);
	}
	fclose(f);
	return 0;
}

int	HTQLTagSelection::setVariableValues(HTQLScope* CurrTag){
	ReferLink* var=Vars;
	char* p;
	unsigned int len;
	HTQLItem item(0);
	tStack* refvar;
	while (var){
		if (ReferVars){
			int from_ref=false;
			for (refvar=ReferVars; refvar; refvar=refvar->Next){
				if (!var->Name.Cmp(refvar->Key, strlen(refvar->Key), false)){
					var->Value = refvar->Value;
					var->Data = CurrTag->SourceOffset;
					from_ref = true;
					break;
				}
			}
			if (from_ref) {
				var = var->Next;
				continue;
			}
		}
		if (!var->Name.Cmp("tn", 2, false)){
			if (Tag.TagType == HTQLTag::tagHTML|| Tag.TagType == HTQLTag::tagXML){
				p=TagOperation::getLabel((const char*) CurrTag->S.P, &len);
				var->Value.Set(p, len, true);
				var->Data = CurrTag->SourceOffset + (p-CurrTag->S.P);
			}else{
				var->Value.Set(Tag.S.P, Tag.S.L, true);
				var->Data = CurrTag->SourceOffset;
			}

		} else if (!var->Name.Cmp("tx",2, false)){ //text between start-tag and end-tag
			setTagVal(CurrTag, &item, -1);
			var->Value.Set(item.Data.P, item.Data.L, true);
			var->Data = item.SourceOffset;

		} else if (!var->Name.Cmp("hx",2, false)){// = ht+xx: (<tag>..</tag>..)<>
			long len=0;
			if (CurrTag->EnclosedTag && CurrTag->EnclosedTag->NextTag){
				len = CurrTag->EnclosedTag->NextTag->S.P - CurrTag->S.P;
			}else if (CurrTag->EnclosedTag){
				len=(CurrTag->EnclosedTag->S.P-CurrTag->S.P)+CurrTag->EnclosedTag->S.L;
				for (p=CurrTag->EnclosedTag->S.P+CurrTag->EnclosedTag->S.L; *p && *p!='<'; p++) len++;
			}else{
				len=CurrTag->S.L;
				for (p=CurrTag->S.P+CurrTag->S.L; *p && *p!='<'; p++) len++;
			}
			var->Value.Set(CurrTag->S.P, len, true);
			var->Data = CurrTag->SourceOffset;
		} else if (!var->Name.Cmp("xx",2, false)){//text after the end-tag
			long len=0;
			if (CurrTag->EnclosedTag && CurrTag->EnclosedTag->NextTag){
				HTQLScope* next_tag=nextTag(CurrTag->EnclosedTag);
				if (next_tag){
					len = next_tag->S.P - CurrTag->EnclosedTag->S.P;
					if (len >= CurrTag->EnclosedTag->S.L) 
						len -= CurrTag->EnclosedTag->S.L;
					else 
						len=0;
				}else{
					len=0;
					for (p=CurrTag->EnclosedTag->S.P+CurrTag->EnclosedTag->S.L; *p && *p!='<'; p++) len++;
				}
				var->Value.Set(CurrTag->EnclosedTag->S.P+CurrTag->EnclosedTag->S.L, len, true);
				var->Data = CurrTag->EnclosedTag->SourceOffset + CurrTag->EnclosedTag->S.L;
			}else if (CurrTag->EnclosedTag){
				len=0;
				for (p=CurrTag->EnclosedTag->S.P+CurrTag->EnclosedTag->S.L; *p && *p!='<'; p++) len++;
				var->Value.Set(CurrTag->EnclosedTag->S.P+CurrTag->EnclosedTag->S.L, len, true);
				var->Data = CurrTag->EnclosedTag->SourceOffset + CurrTag->EnclosedTag->S.L;
			}else{
				len=0;
				for (p=CurrTag->S.P+CurrTag->S.L; *p && *p!='<'; p++) len++;
				var->Value.Set(CurrTag->S.P+CurrTag->S.L, len, true);
				var->Data = CurrTag->SourceOffset + CurrTag->S.L;
			}
		} else if (!var->Name.Cmp("fx",2, false)){//text after the start-tag and before the next tag
			long len = 0;
			HTQLScope* next_tag = nextTag(CurrTag);  
			if (next_tag){
				len=next_tag->S.P - CurrTag->S.P;
				if (len >= CurrTag->S.L) 
					len -= CurrTag->S.L;
				else 
					len=0;
			}else{
				len=0;
				for (p=CurrTag->S.P+CurrTag->S.L; *p && *p!='<'; p++) len++;
			}
			var->Value.Set(CurrTag->S.P+CurrTag->S.L, len, true);
			var->Data = CurrTag->SourceOffset + CurrTag->S.L;
		} else if (!var->Name.Cmp("px",2, false)){//text before the start-tag
			long len = 0;
			HTQLScope* prev_tag = prevTag(CurrTag, true); 
			if (prev_tag){
				len=CurrTag->S.P - prev_tag->S.P;
				if (len >= prev_tag->S.L) 
					len -= prev_tag->S.L;
				else 
					len=0;
				var->Value.Set(prev_tag->S.P+prev_tag->S.L, len, true);
				var->Data = prev_tag->SourceOffset + prev_tag->S.L;
			}else{
				var->Value.Set(CurrTag->S.P, 0, true);
				var->Data = CurrTag->SourceOffset;
			}

		} else if (!var->Name.Cmp("ht",2, false)){
			setTagVal(CurrTag, &item, true);
			var->Value.Set(item.Data.P, item.Data.L, true);
			var->Data = item.SourceOffset;

		} else if (!var->Name.Cmp("st",2, false)){
			if (CurrTag->TagType == HTQLTagDataSyntax::synQL_DATA){
				HTQLScope* prev_tag = prevTag(CurrTag, false); 
				if (!prev_tag) prev_tag = CurrTag; 
				var->Value.Set(prev_tag->S.P, prev_tag->S.L, true);
				var->Data = prev_tag->SourceOffset;

			}else{
				var->Value.Set(CurrTag->S.P, CurrTag->S.L, true);
				var->Data = CurrTag->SourceOffset;
			}
		} else if (!var->Name.Cmp("tt",2, false)){ // includes st, et, or data
			if (CurrTag->TagType == HTQLTagDataSyntax::synQL_DATA){
				var->Value.Set(CurrTag->S.P, CurrTag->E.P-CurrTag->S.P, true);
			}else{
				var->Value.Set(CurrTag->S.P, CurrTag->S.L, true);
			}
			var->Data = CurrTag->SourceOffset;

		} else if (!var->Name.Cmp("et",2, false)){
			var->Value.Set(CurrTag->E.P, CurrTag->E.L, true);
			if (CurrTag->E.P > CurrTag->S.P){
				var->Data = CurrTag->SourceOffset + (CurrTag->E.P - CurrTag->S.P);
			}else if (CurrTag->EnclosedTag){
				var->Data = CurrTag->EnclosedTag->SourceOffset;
			}else{
				var->Data = ((unsigned long) Data.L<CurrTag->SourceOffset + strlen(CurrTag->S.P))?Data.L:CurrTag->SourceOffset + strlen(CurrTag->S.P) ;
			}


		} else if (!var->Name.Cmp("th",2, false) && TagOperation::isTag(CurrTag->S.P, "td") ){ //table header on the top
			int count=1;
			for (HTQLScope* previous_td=CurrTag->PreviousTag; previous_td; previous_td=previous_td->PreviousTag){
				if (TagOperation::isTag(previous_td->S.P, "td")) count++; 
				else if (TagOperation::isTag(previous_td->S.P, "tr") || TagOperation::isTag(previous_td->S.P, "table")) break;

				if (previous_td->EnclosedTag && previous_td->EnclosedTag->SourceOffset<previous_td->EnclosedTag->SourceOffset)  
					previous_td=previous_td->EnclosedTag;
			}
			HTQLItem* tableitem=ParentItem;
			while (tableitem && !TagOperation::isTag(tableitem->S.P, "Table")) tableitem=tableitem->ParentItem;

			if (tableitem){
				if (!tableitem->FieldName.P || !strchr(tableitem->FieldName.P, '\n')){
					//data to parse, save header in FieldName and separate them by \n
					HTQL ql;
					ql.setSourceData(tableitem->Data.P, tableitem->Data.L,false);
					ql.setQuery("<th> {a=&txstr; b=:colspan} ");
					if (ql.isEOF())
						ql.setQuery("<tr>1.<td> {a=&txstr; b=:colspan} ");
					
					for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
						int colspan=1;
						p=ql.getValue(2); 
						if (p) colspan=atoi(p); 
						tableitem->FieldName+="\n";
						tableitem->FieldName+=ql.getValue(1);
						for (int i=1; i<colspan; i++){
							tableitem->FieldName+="\n";
							tableitem->FieldName+=ql.getValue(1);
						}
					}
					tableitem->FieldName+="\n";
				}
				p=strchr(tableitem->FieldName.P, '\n');
				for (int i=1; i<count; i++){ //find the right header
					if (p && p[1]) p=strchr(p+1, '\n');
				}
				if (p){ //copy the header value
					char* p1=strchr(p+1, '\n');
					if (p1) var->Value.Set(p+1, p1-p-1,true);
					else var->Value.Set(p+1, strlen(p+1), true);
				}else{
					var->Value="";
				}
				var->Data=tableitem->SourceOffset; //not exact offset, modify later
			}else{
				var->Value="";
			}
		} else if (!var->Name.Cmp("th",2, false) && TagOperation::isTag(CurrTag->S.P, "tr") ){ //table header on the left
			var->Value="";
			const char* to_find[]={"</th>","</td>","<table>", "<tr>", "</table>","</tr>", 0};
			if (CurrTag && CurrTag->NextTag && (TagOperation::isTag(CurrTag->NextTag->S.P, "<th>") || TagOperation::isTag(CurrTag->NextTag->S.P, "<td>")) ){
				HTQLScope* td=CurrTag->NextTag; //start
				HTQLScope* td1=0; //end
				for (td1=td->NextTag; td1 && td1->SerialNo<CurrTag->EnclosedTag->SerialNo && TagOperation::isTags(td1->S.P, to_find)<0; td1=td1->NextTag); //end
				if (td1){
					var->Value.Set(td->S.P+td->S.L, td1->S.P - td->S.P - td->S.L, true);
				}else if (td->E.P){
					var->Value.Set(td->S.P+td->S.L, td->E.P - td->S.P - td->S.L, true);
				}else if (CurrTag->E.P){
					var->Value.Set(td->S.P+td->S.L, CurrTag->E.P - td->S.P - td->S.L, true);
				}else{
					var->Value.Set(td->S.P+td->S.L, Data.P+Data.L - td->S.P - td->S.L, true);
				}
				var->Data = td->SourceOffset + td->S.L;
			}
			/*
			HTQLScope* td=0;
			int found=false;
			const char* to_find[]={"<th>","<td>",0};
			var->Value="";
			for (int i=0; !found && to_find[i]; i++){
				for (td=CurrTag->NextTag; td && td->SerialNo<CurrTag->EnclosedTag->SerialNo; td=td->NextTag){
					if (TagOperation::isTag(td->S.P, to_find[i]) ){
						if (td->E.P){
							var->Value.Set(td->S.P+td->S.L, td->E.P - td->S.P - td->S.L, true);
						}else if (CurrTag->E.P){
							var->Value.Set(td->S.P+td->S.L, CurrTag->E.P - td->S.P - td->S.L, true);
						}else{
							var->Value.Set(td->S.P+td->S.L, Data.P+Data.L - td->S.P - td->S.L, true);
						}
						var->Data = td->SourceOffset + td->S.L;
						found=true;
						break;
					}
				}
			}*/
		}else if (Tag.TagType == HTQLTag::tagHTML|| Tag.TagType == HTQLTag::tagXML){
			HTQLAttrDataSyntax AttrSyn;
			AttrSyn.setSentence(CurrTag->S.P, &CurrTag->S.L, false);
			ReferData name, name_keep;
			while (AttrSyn.Type != QLSyntax::synQL_UNKNOW &&
				AttrSyn.Type != QLSyntax::synQL_END 
				){
				if (AttrSyn.Type == QLSyntax::synQL_WORD ){
					name.Set(AttrSyn.Sentence+AttrSyn.Start, AttrSyn.StartLen, true);
					tStrOp::replaceInplace(name.P, ":", "_");
					tStrOp::replaceInplace(name.P, "-", "_");
					if (!var->Name.Cmp(name.P, name.L, Options.Sen)) {
						name_keep.Set(AttrSyn.Sentence+AttrSyn.Start, AttrSyn.StartLen, false);
						break;
					}
				}
				AttrSyn.match();
			}
			if (AttrSyn.Type == QLSyntax::synQL_WORD){
				AttrSyn.match();
				if (AttrSyn.Type!=QLSyntax::synQL_EQ){ //no value, such as 'selected'
					var->Value.Set(name_keep.P, name_keep.L, true);
					var->Data=CurrTag->SourceOffset + (name_keep.P-AttrSyn.Sentence); 
				}else{ //general tag attribute
					AttrSyn.match();// synQL_EQ;
					if (AttrSyn.Type == QLSyntax::synQL_WORD){
						var->Value.Set(AttrSyn.Sentence+AttrSyn.Start, AttrSyn.StartLen, true);
						var->Data = CurrTag->SourceOffset + AttrSyn.Start;
					}else if (AttrSyn.Type == QLSyntax::synQL_STRING){
						if (AttrSyn.Sentence[AttrSyn.Start] == '\'' || AttrSyn.Sentence[AttrSyn.Start] == '"'){
							var->Value.Set(AttrSyn.Sentence+AttrSyn.Start+1, AttrSyn.StartLen-2, true);
							var->Data = CurrTag->SourceOffset + AttrSyn.Start+1;
						}else{
							var->Value.Set(AttrSyn.Sentence+AttrSyn.Start, AttrSyn.StartLen, true);
							var->Data = CurrTag->SourceOffset + AttrSyn.Start;
						}
					}else{
						var->Value.Set(AttrSyn.Sentence+AttrSyn.Start, 0, true);
						var->Data = CurrTag->SourceOffset + AttrSyn.Start;
					}
				}
			}else{
				//var->Value.Set(AttrSyn.Sentence+AttrSyn.Start, 0, true);
				var->Value.reset(); //attribute does not exist
				var->Data = CurrTag->SourceOffset + AttrSyn.Start;
			}
		}
		var=var->Next;
	}
	return 0;
}
HTQLScope*	HTQLTagSelection::nextTag(HTQLScope* CurrTag){
	HTQLScope* next_tag = CurrTag->NextTag; 
	while (next_tag && !TagOperation::isTag(next_tag->S.P)) next_tag=next_tag->NextTag;
	return next_tag; 
}
HTQLScope*	HTQLTagSelection::prevTag(HTQLScope* CurrTag, int keep_last){
	HTQLScope* prev_tag = CurrTag->PreviousTag; 
	while (prev_tag && !TagOperation::isTag(prev_tag->S.P) && (!keep_last || prev_tag->PreviousTag ) )
		prev_tag=prev_tag->PreviousTag;
	return prev_tag;
}

HTQLScope* HTQLTagSelection::searchNextMatchingAt(HTQLScope* curr_tag, ReferData* tag_name){
	HTQLScope* current_tag=curr_tag;
	if (current_tag) current_tag = current_tag->NextTag;
	while (current_tag && (
			(!tag_name->Cmp("st",2,false) && current_tag->TagType != HTQLTagDataSyntax::synQL_START_TAG)
			|| (!tag_name->Cmp("et",2,false) && current_tag->TagType != HTQLTagDataSyntax::synQL_END_TAG)
			|| (!tag_name->Cmp("tt",2,false) && current_tag->TagType != HTQLTagDataSyntax::synQL_END_TAG && current_tag->TagType != HTQLTagDataSyntax::synQL_START_TAG)
			|| (!tag_name->Cmp("tx",2,false) && current_tag->TagType != HTQLTagDataSyntax::synQL_DATA)
			)
		){
		current_tag = current_tag->NextTag;
	}
	return current_tag;
}

HTQLScope* HTQLTagSelection::searchNextTagNoRecur(HTQLScope* curr_tag, ReferData* tag_name, int tag_type, int noembedding, ReferData* last_tagname, int noiframe, int iswildmatching){
	HTQLScope* current_tag=curr_tag;
	if (current_tag) current_tag = current_tag->NextTag;
	const char* noiframe_tags[]={"<iframe>", "<noscript>", 0};
	while (current_tag && 
		(current_tag->TagType != HTQLTagDataSyntax::synQL_START_TAG 
		|| !isMatchedHtmlTag(current_tag, tag_name,iswildmatching) ) 
		){
		if (tag_type == HTQLTag::tagHTML && current_tag->EnclosedTag && current_tag->TagType == HTQLTagDataSyntax::synQL_START_TAG 
			&& ( (noiframe && TagOperation::isTags(current_tag->S.P, noiframe_tags)>=0)
				||TagOperation::isTag(current_tag->S.P, "script")
				|| (noembedding && TagOperation::isTag(current_tag->S.P, last_tagname->P)) )
			){
			current_tag=current_tag->EnclosedTag->NextTag;
		}else{
			current_tag = current_tag->NextTag;
		}
	}
	return current_tag;
}
HTQLScope* HTQLTagSelection::searchNextPlainTagNoRecur(HTQLScope* curr_tag, ReferData* tag_name, int tag_type, TagOptions* options, ReferData* last_tagname){
	HTQLScope* current_tag=curr_tag;
	if (current_tag) current_tag = current_tag->NextTag;
	while (current_tag && 
		(current_tag->TagType != HTQLTagDataSyntax::synQL_START_TAG 
		|| tag_name->Cmp(&current_tag->S, options?options->Sen:false) ) 
		){
		current_tag = current_tag->NextTag;
	}
	return current_tag;
}
int HTQLTagSelection::isMatchedHtmlTag(HTQLScope* curr_tag, ReferData* tag_name, int iswildmatching){
	if (iswildmatching){
		return true; //match any tag for now
	}else{
		return TagOperation::isTag(curr_tag->S.P, tag_name->P); 
	}
}

HTQLScope* HTQLTagSelection::searchNextTag(HTQLScope* curr_tag, ReferData* tag_name, int is_html, int case_sensitive, int noembedding, ReferData* last_tagname, int noiframe, int iswildmatching){
	HTQLScope* current_tag=curr_tag;
	if (is_html){
		if (current_tag 
			&& current_tag->TagType == HTQLTagDataSyntax::synQL_START_TAG
			&& current_tag != current_tag->EnclosedTag
			&& current_tag->EnclosedTag 
			&& isMatchedHtmlTag(current_tag, tag_name,iswildmatching) //continue from last search, current tag is a sibling tag of the tag_name
					//if the parent tag same as the child tag, parent need to advance to the next tag before calling this
			){ //skip itself
			current_tag = current_tag->EnclosedTag;
		}else if (current_tag){ //move to the next tag
			current_tag = current_tag->NextTag;
		}
		const char* noiframe_tags[]={"<iframe>", "<noscript>", 0};
		while (current_tag && 
			(current_tag->TagType != HTQLTagDataSyntax::synQL_START_TAG 
			||!isMatchedHtmlTag(current_tag, tag_name,iswildmatching) ) 
			){
			if ( current_tag->TagType == HTQLTagDataSyntax::synQL_START_TAG
				&& current_tag->EnclosedTag 
				&& current_tag != current_tag->EnclosedTag 
				&&	((noembedding && TagOperation::isTag(current_tag->S.P, last_tagname->P) )
					 ||(noiframe && TagOperation::isTags(current_tag->S.P, noiframe_tags)>=0 )
					)
				){ //for non-embedding mode, skip the child of the last tag
				current_tag = current_tag->EnclosedTag;
			}
			current_tag = current_tag->NextTag;
		}
		if (current_tag && !current_tag->EnclosedTag){
			HTQLScope* NextScope=current_tag->NextTag;
			while (NextScope && 
				(NextScope->TagType != HTQLTagDataSyntax::synQL_START_TAG 
				||!isMatchedHtmlTag(NextScope, tag_name,iswildmatching) ) 
				){
				if (current_tag->TagType == HTQLTagDataSyntax::synQL_START_TAG
					&& (noembedding && TagOperation::isTag(NextScope->S.P, last_tagname->P))
					) 
					break;
				NextScope = NextScope->NextTag;
			}
			if (NextScope) current_tag->EnclosedTag = NextScope;
		}
	}else {
		if (current_tag && 
			current_tag->TagType == HTQLTagDataSyntax::synQL_START_TAG &&
			!current_tag->S.Cmp(tag_name->P,tag_name->L, case_sensitive) &&
			current_tag->EnclosedTag ){
			current_tag = current_tag->EnclosedTag;
		}else if (current_tag){
			current_tag = current_tag->NextTag;
		}
		while (current_tag && 
			(current_tag->TagType != HTQLTagDataSyntax::synQL_START_TAG 
			||current_tag->S.Cmp(tag_name->P,tag_name->L, case_sensitive) )
			){
			current_tag = current_tag->NextTag;
		}
		if (current_tag && !current_tag->EnclosedTag){
			HTQLScope* NextScope=current_tag->NextTag;
			while (NextScope && 
				(NextScope->TagType != HTQLTagDataSyntax::synQL_START_TAG 
				||NextScope->S.Cmp(tag_name->P,tag_name->L, case_sensitive) )
				){
				NextScope = NextScope->NextTag;
			}
			if (NextScope) current_tag->EnclosedTag = NextScope;
		}
	}
	return current_tag;
}


HTQLScope* HTQLTagSelection::movetoNextTag(){
	if (!TagScope) return 0;
	/*if (Options.Sep && TagScope == CurrentTagScope && 
		TagScope->NextTag && TagScope->NextTag->SerialNo==0
		){
		CurrentTagScope = TagScope->NextTag;
		return CurrentTagScope;
	}*/
	if (Options.WildMatching == Options.matAT){
		CurrentTagScope = searchNextMatchingAt(CurrentTagScope, &Tag.S);
	} else if (Tag.TagType == HTQLTag::tagHTML|| Tag.TagType == HTQLTag::tagXML){
		if (Options.Recur)
			CurrentTagScope = searchNextTag(CurrentTagScope, &Tag.S, true, false, Options.NoEmbedding, &LastTag.S, Options.NoIFrame, Options.WildMatching);
		else
			CurrentTagScope = searchNextTagNoRecur(CurrentTagScope, &Tag.S, Tag.TagType, Options.NoEmbedding, &LastTag.S, Options.NoIFrame, Options.WildMatching);
	}else if (Tag.TagType == HTQLTag::tagPLAIN){
		if (Options.Recur)
			CurrentTagScope = searchNextTag(CurrentTagScope, &Tag.S, false, Options.Sen, 0, 0, true, Options.WildMatching);
		else
			CurrentTagScope = searchNextPlainTagNoRecur(CurrentTagScope, &Tag.S, Tag.TagType, &Options, &LastTag.S);

	}else{
		return 0;
	}
	return CurrentTagScope;
}

int HTQLTagSelection::parseTagScope(){
	if (TagScope) {
		delete TagScope;
		TagScope = 0;
	}
	if (!Data.P) return SUCCESS;
	
	if (Tag.TagType == HTQLTag::tagHTML|| Tag.TagType == HTQLTag::tagXML){
		parseHTMLScope();
	}else if (Tag.TagType == HTQLTag::tagPLAIN){
		parsePLAINScope();
	}
	/* not sure why the Tag needs to be set to the parsed tag now!  */
	if (!Tag.S.P && TagScope && TagScope->S.P && TagOperation::isTag(TagScope->S.P) ){
		unsigned int len=0;
		char*p=TagOperation::getLabel(TagScope->S.P, &len);
		Tag.S.Set(p, len, true);
	} 

	/*if (Options.Sep){
		CurrentTagScope = TagScope;
		movetoNextTag();
		if (CurrentTagScope){
			HTQLScope* newTag = new HTQLScope;
			newTag->S.Set(TagScope->S.P, 0, false);
			newTag->EnclosedTag = CurrentTagScope;
			newTag->NextTag = TagScope->NextTag;
			TagScope->NextTag->PreviousTag=newTag;
			newTag->PreviousTag=TagScope;
			TagScope->NextTag=newTag;
		}
	}*/
	return 0;
}

int HTQLTagSelection::linkPairTag(HTQLScope* PreviousTag, int isPlain){
	HTQLScope* newTag;
	unsigned int LabelLen=0;
	char* Label=0;
	if (!isPlain){
		LabelLen=0;
		Label=TagOperation::getLabel(PreviousTag->S.P, &LabelLen);
	}else{
		Label=PreviousTag->S.P;
		LabelLen = PreviousTag->S.L;
	}
	for (newTag=PreviousTag->PreviousTag; newTag; newTag=newTag->PreviousTag) {
		if (newTag->TagType == HTQLTagDataSyntax::synQL_END_TAG) continue;
		// newTag is a start-tag

		if (newTag->E.P||newTag->EnclosedTag) continue;
		// newTag has no assigned matched tag


		if (LabelLen){
			if (isPlain) break;
			else if (!TagOperation::isTag(newTag->S.P, Label) ) continue; 
		}
		// Label is null or is same as newTag;

		break;
	}
	if (newTag) {
		newTag->E.Set(PreviousTag->S.P, PreviousTag->S.L, false);
		newTag->EnclosedTag = PreviousTag;
		PreviousTag->EnclosedTag = newTag;
	}
	return 0;
}

int HTQLTagSelection::linkUnclosedTableTag(char*Source, long length, HTQLScope* start){
	HTQLScope* tag = start;
	while (tag){
		if (tag->S.P && !tag->E.P && tag->NextTag && TagOperation::isTag(tag->S.P, "TABLE") && TagOperation::isTag(tag->NextTag->S.P, "TBODY")){
			tag->E.Set(tag->NextTag->E.P+tag->NextTag->E.L, 0, false);
		}
		tag=tag->NextTag;
	}
	return 0;
}
int HTQLTagSelection::linkUnclosedTag(char*Source, long length, HTQLScope* start, char* tagname){
	HTQLScope* tag = start;
	while (tag->NextTag) tag=tag->NextTag;
	char* last_offset = Source+length;
	while (tag){
		if (tag->S.P && !tag->E.P && TagOperation::isTag(tag->S.P, tagname)){
			tag->E.Set(last_offset, 0, false);
			last_offset = tag->S.P;
		}
		tag=tag->PreviousTag;
	}
	return 0;
}
int HTQLTagSelection::linkUnclosedTag(char*Source, long length, HTQLScope* start, char* tagname, char* parent_tagname){
	HTQLScope* tag = start;
	while (tag->NextTag) tag=tag->NextTag;
	char* last_offset = Source+length;
	while (tag){
		if (tag->S.P && TagOperation::isTag(tag->S.P, parent_tagname)){
			//if (!tag->E.P ){
			//	tag->E.Set(last_offset, 0, false);
			//	tag->EnclosedTag=0;
			//}
			for (HTQLScope* tag1=tag->NextTag; tag1&&tag1->S.P<tag->E.P; tag1=tag1->NextTag){
				if (tag1->S.P && TagOperation::isTag(tag1->S.P, tagname)){
					//if (!tag1->E.P || tag1->E.P>tag->E.P){ //force to enclose before the parent tag? not sure
					if (!tag1->E.P ){
						tag1->E.Set(tag->E.P, 0, false);
						tag1->EnclosedTag=0;
					}
				}
			}
			last_offset = tag->S.P;
		}
		tag=tag->PreviousTag;
	}
	return 0;
}
int HTQLTagSelection::parsePLAINScope(){
	HTQLScope ** pTags = &TagScope;
	HTQLScope*  newTag;
	HTQLScope* PreviousTag = NULL;
	int SerialNo=0;

	if (TagScope){
		delete TagScope;
		TagScope=0;
	}
	newTag=new HTQLScope;
	if (!newTag) return tagMEMORY;
	newTag->S.Set(Data.P, 0, false);
	newTag->SourceOffset = SourceOffset;
	newTag->E.Set(Data.P+Data.L, 0, false);
	newTag->SerialNo = ++ SerialNo;
	newTag->PreviousTag = PreviousTag;
	newTag->TagType = HTQLTagDataSyntax::synQL_START_TAG;
	PreviousTag = newTag;
	*pTags = newTag;
	pTags = &(newTag->NextTag);

	long pos=0;
	size_t i;
	int type;
	int forEnd=false;
	int has_start=0, has_end=0; 
	while (pos < Data.L){
		type=HTQLTagDataSyntax::synQL_DATA;
		i=0;
		if (forEnd){
			if (Tag.E.Cmp(Data.P+pos, Tag.E.L, Options.Sen)==0){
				type=HTQLTagDataSyntax::synQL_END_TAG;
				i=Tag.E.L;
				has_end=1;
			}
		}else{
			if (Tag.S.Cmp(Data.P+pos, Tag.S.L, Options.Sen)==0){
				type=HTQLTagDataSyntax::synQL_START_TAG;
				i=Tag.S.L;
				has_start=1; 
			}
		}

		if (type!= HTQLTagDataSyntax::synQL_DATA){
			newTag=new HTQLScope;
			if (!newTag) return tagMEMORY;
			newTag->S.Set(Data.P + pos, i, false);
			newTag->SourceOffset = SourceOffset + pos;
			newTag->TagType = type; // synQL_END_TAG or synQL_START_TAG
			newTag->SerialNo = ++ SerialNo;
			newTag->PreviousTag = PreviousTag;
			PreviousTag = newTag;

			*pTags = newTag;
			pTags = &(newTag->NextTag);

			//if (type == HTQLTagDataSyntax::synQL_END_TAG && !Options.Sep){
			if (type == HTQLTagDataSyntax::synQL_END_TAG){
				linkPairTag(PreviousTag, true);
			}
		}

		pos+=i; 

		forEnd = !forEnd;
		if (forEnd==0){
			if (!has_start && !has_end){
				pos+=1; 
			}
			has_start=0; 
			has_end=0; 
		}
		//if (!forEnd || type!= HTQLTagDataSyntax::synQL_DATA){
		//	pos+=i;
		//}
	}
	if (Tag.E.P){ //ensure there is a enclosed tag
		for (newTag=TagScope; newTag; newTag=newTag->NextTag){
			if (newTag->TagType == HTQLTagDataSyntax::synQL_START_TAG && !newTag->EnclosedTag){
				newTag->TagType=HTQLTagDataSyntax::synQL_DATA; 
			}
		}
	}
	return 0;
}
int HTQLTagSelection::parseHTMLScope(int include_comment){
	HTQLScope ** pTags = &TagScope;
	HTQLScope*  newTag;
	HTQLScope* PreviousTag = NULL;
	int SerialNo=0;

	if (TagScope){
		delete TagScope;
		TagScope=0;
	}
	newTag=new HTQLScope;
	if (!newTag) return tagMEMORY;
	newTag->S.Set(Data.P, 0, false);
	newTag->E.Set(Data.P+Data.L, 0, false);
	newTag->SourceOffset = SourceOffset;
	newTag->SerialNo = ++ SerialNo;
	newTag->PreviousTag = PreviousTag;
	newTag->TagType = HTQLTagDataSyntax::synQL_START_TAG;
	PreviousTag = newTag;
	*pTags = newTag;
	pTags = &(newTag->NextTag);
	
	HTQLTagDataSyntax DataSyntax;
	DataSyntax.setSentence(Data.P, &Data.L, false);
	while (DataSyntax.Type != QLSyntax::synQL_END){
		if ( // DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA || 
			(!include_comment && DataSyntax.Type == HTQLTagDataSyntax::synQL_COMMENT)
			){
			DataSyntax.match();
			continue;
		}
		newTag=new HTQLScope;
		if (!newTag) return tagMEMORY;
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA){
			newTag->S.Set(DataSyntax.Sentence+DataSyntax.Start, 0, false);
			newTag->E.Set(DataSyntax.Sentence+DataSyntax.Start+DataSyntax.StartLen, 0, false);
		}else{
			newTag->S.Set(DataSyntax.Sentence+DataSyntax.Start, DataSyntax.StartLen, false);
		}
		newTag->SourceOffset = SourceOffset + DataSyntax.Start;
		newTag->TagType = DataSyntax.Type; // synQL_END_TAG or synQL_START_TAG
		newTag->SerialNo = ++ SerialNo;
		newTag->PreviousTag = PreviousTag;
		PreviousTag = newTag;

		*pTags = newTag;
		pTags = &(newTag->NextTag);

		//if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG && !Options.Sep){
		if (DataSyntax.Type == HTQLTagDataSyntax::synQL_END_TAG){
			linkPairTag(PreviousTag, false);
		}else if (DataSyntax.Type == HTQLTagDataSyntax::synQL_DATA){
			PreviousTag->EnclosedTag = PreviousTag;
		}else if (//!Options.Sep && //DataSyntax.IsXML &&
				DataSyntax.Type == HTQLTagDataSyntax::synQL_START_TAG && 
				DataSyntax.XMLTagType == HTQLTagDataSyntax::synQL_XML_SINGLE_TAG)
			{
			PreviousTag->E.Set(PreviousTag->S.P+PreviousTag->S.L, 0, false);
			PreviousTag->EnclosedTag = PreviousTag;
		}

		DataSyntax.match();
	}

	if (!DataSyntax.IsXML){
		linkUnclosedTag(Data.P, Data.L, TagScope, "P");
		linkUnclosedTag(Data.P, Data.L, TagScope, "TR", "TABLE");
		linkUnclosedTag(Data.P, Data.L, TagScope, "TD", "TR");
		linkUnclosedTag(Data.P, Data.L, TagScope, "TABLE", "TD");
		//linkUnclosedTag(Data.P, Data.L, TagScope, "TABLE");
	}
	//dumpHTMLScope(TagScope, "c:\\scope.txt");
	return 0;
}

HTQLParser::HTQLParser(){
	Data = 0;
	Results=0;
	MidResults=0;
	CurrentRecord=0;
	TagSelections=0;
	IsMidResult=false;
	HtqlFunctions.setCaseSensitivity(false);
	QueryPatternNum=0;

	addHtmlQLFunctions();

	FunctionCurrentItem=0;
	FunctionParameters=0;
	CurrentFunction=0;
}

int HTQLParser::addHtmlQLFunctions(){
	addHtqlFunction("help", (void *)functionHelp, "Help of functions");
	addHtqlFunction("about", (void *)functionAbout, "About HTQL");
	addHtqlFunction("tx", (void *)functionTX, "Translate HTML encoded chars to the original chars");
	addHtqlFunction("txstr", (void *)functionTxStr, "Translate HTML encoded chars to text string");
	addHtqlFunction("html_encode", (void *)toHtmlPrintable, "Encode special HTML chars to printable codes");
	addHtqlFunction("html_decode", (void *)functionDecodeHtml, "Decode special HTML chars");
	addHtqlFunction("url_encode", (void *)functionEncodeUrl, "Encode special URL chars to special codes");
	addHtqlFunction("url_decode", (void *)functionDecodeUrl, "Decode special URL chars");
	addHtqlFunction("toupper", (void *)toUpper, "Translate to uppercase chars");
	addHtqlFunction("tolower", (void *)toLower, "Translate to lowercase chars");
	addHtqlFunction("get_url", (void*) functionGetUrl, "Get the url of a link");
	addHtqlFunction("url", (void*) functionUrl, "Translate to full URL address");
	addHtqlFunction("url_all", (void*) functionUrlAll, "Translate all urls to full URL address");
	addHtqlFunction("skip_header", (void*) functionSkipHeader, "Skip HTTP header");
	addHtqlFunction("after", (void*) functionAfter, "text after a pattern; PARA 1: pattern; PARA 2: pattern index");
	addHtqlFunction("before", (void*) functionBefore, "text before a pattern; PARA 1: pattern; PARA 2: pattern index");
	addHtqlFunction("get_number", (void*) functionGetNumber, "search for number; PARA 1: index of number (1 base)");
	addHtqlFunction("get_date", (void*) functionGetDate, "search for date; PARA 1: index of date (1 base)");
	addHtqlFunction("get_csvfield", (void*) functionGetCsvField, "search for csv field; PARA 1: field seporator; PARA 2: index of csv field (1 base)");
	addHtqlFunction("trim", (void*) functionTrim, "trim space at the begining and the end; PARA 1: list of characters to be trimmed; PARA2: positions to trim, default '101' ");
	addHtqlFunction("char_replace", (void*) functionCharReplace, "replace each char in PARA1 to the corresponding char in PARA2; PARA 1: list of characters to be replaced; "
		"PARA 2: list of the corresponding new chars to replace the old chars. if it is not listed, the old char is just deleted.");
	HTQLFunctionUpdateStruct::addUpdateFunction(this, HTQLFunctionUpdateStruct::funINSERT_BEFORE, "INSERT", "insert from the current position; PARA 1: insert expression");
	HTQLFunctionUpdateStruct::addUpdateFunction(this, HTQLFunctionUpdateStruct::funINSERT_AFTER, "INSERT_AFTER", "insert after the current tag; PARA 1: insert expression");
	HTQLFunctionUpdateStruct::addUpdateFunction(this, HTQLFunctionUpdateStruct::funDELETE, "DELETE", "delete the current tag;");
	HTQLFunctionUpdateStruct::addUpdateFunction(this, HTQLFunctionUpdateStruct::funUPDATE, "UPDATE", "update the current tag; PARA 1: updated expression");
	HTQLFunctionUpdateStruct::addUpdateFunction(this, HTQLFunctionUpdateStruct::funUPDATE, "REPLACE", "replace the current tag; PARA 1: replaced expression");
	HTQLFunctionUpdateStruct::addUpdateFunction(this, HTQLFunctionUpdateStruct::funSET_ATTRIBUTE, "SET_ATTRIBUTE", "set attribute of the current tag; PARA 1: name; PARA 2: value");
	HTQLFunctionUpdateStruct::addUpdateFunction(this, HTQLFunctionUpdateStruct::funDELETE_ATTRIBUTE, "DELETE_ATTRIBUTE", "delete attribute of the current tag; PARA 1: name;");
	HTQLFunction* update_function=0;
	addHtqlFunction("SAVE", 0, "Save source data into file; PARA 1: filename", &update_function);
	if (update_function){
		update_function->FunComplete = functionSaveSourceFile;
		update_function->FunItem = 0; 
	}
	addHtqlFunction("remove_header", (void*) functionRemoveHeader, "Remove HTTP header",  &update_function);
	if (update_function){
		update_function->FunComplete = functionRemoveHeader; 
		update_function->FunItem = 0; 
	}
	addHtqlFunction("set_source", (void*) functionSetSource, "Set source data",  &update_function);
	if (update_function){
		update_function->FunComplete = functionSetSource; 
		update_function->FunItem = 0; 
	}
	addHtqlFunction("tuple_count", (void*) functionTupleCount, "Get the number of tuples",  &update_function);
	if (update_function){
		update_function->FunComplete = functionTupleCount; 
		update_function->FunItem = 0; 
	}
	return 0;
}

HTQLParser::~HTQLParser(){
	reset();
	resetHtqlFunctions();
	resetGlobalVariables();
}

void HTQLParser::reset(){
#if (ERRORLOG==3)
	Log::add(LOGFILE,(long)Data, "HTQLParser::reset()", "delete Data;", __LINE__,__FILE__);
#endif
	if (Data) {
		delete Data;
		Data =0;
	}
#if (ERRORLOG==3)
	Log::add(LOGFILE,(long)Results, "HTQLParser::reset()", "delete Results;", __LINE__,__FILE__);
#endif
	if (Results){
		delete Results;
		Results=0;
	}
#if (ERRORLOG==3)
	Log::add(LOGFILE,(long)TagSelections, "HTQLParser::reset()", "delete TagSelections;", __LINE__,__FILE__);
#endif
	if (TagSelections){
		ReferLink* p=TagSelections;
		while (p){
			if (p->Data == htqlTAG_SELECTION){// is expression at value
				if (p->Value.P) delete (HTQLTagSelection*)p->Value.P;
			}else if (p->Data == htqlHTQL_PARSER){// is expression at value
				if (p->Value.P) delete (HTQLParser*)p->Value.P;
			}
			p=p->Next;
		}
		delete TagSelections;
		TagSelections=0;
	}
#if (ERRORLOG==3)
	Log::add(LOGFILE,(long)MidResults, "HTQLParser::reset()", "delete MidResults;", __LINE__,__FILE__);
#endif
	if (MidResults){
		delete MidResults;
		MidResults=0;
	}
	IsMidResult=false;
	CurrentRecord=0;
	//Sentence.reset();
	//SourceData.reset();
	//SourceUrl.reset();
	QueryPatternNum=0;

	Options.reset();
	Syntax.reset();
	LastTag.reset();
	BaseUrl.reset();
	FunctionCurrentItem=0;
#if (ERRORLOG==3)
	Log::add(LOGFILE,(long)FunctionParameters, "HTQLParser::reset()", "delete FunctionParameters;", __LINE__,__FILE__);
#endif
	resetHtqlFunctionParameters();
	CurrentFunction=0;
}

char* HTQLParser::setData(char* data, long len, int copy){
	if (Data) switchResults();
	if (Data) switchResults();
	if (data && strlen(data)<(unsigned long) len) len=strlen(data);
	SourceData.Set(data, len, copy);
	Data=new HTQLItem(0);
	Data->Data.Set(SourceData.P, SourceData.L, false);
	return Data->Data.P;
}

char* HTQLParser::resetData(){
	if (Results) {
		delete Results;
		Results = 0;
	}
	if (Data){
		delete Data;
		Data=0;
	}
	Data=new HTQLItem(0);
	Data->Data.Set(SourceData.P, SourceData.L, false);
	return Data->Data.P;
}

char* HTQLParser::setSentence(char* sentence, long len, int copy){
	Sentence.Set(sentence, len, copy);
	QueryPatternNum=0;
	return Sentence.P;
}

char* HTQLParser::setSourceUrl(char* url, long len){
	SourceUrl.Set(url, len, true);
	CurrentSourceUrl.Set(SourceUrl.P, SourceUrl.L, false);
	return SourceUrl.P;
}

char* HTQLParser::getSourceUrl(){
	return SourceUrl.P;
}

int HTQLParser::dotSentence(char* sentence, long len, int copy){
	Sentence.Cat(sentence, len);
	Syntax.setSentence(sentence, &len, copy);
	int i=parseSentence();
	if (i<0) return i;
	moveFirst();
	return 0;
}

int HTQLParser::parse(){
	Syntax.setSentence(Sentence.P, &Sentence.L, false);
	if (!Data){
		Data=new HTQLItem(0);
		if (SourceData.P) {
			Data->Data.Set(SourceData.P, SourceData.L, false);
			Data->S.Set(SourceData.P, 0, false);
			Data->E.Set(SourceData.P+SourceData.L, 0, false);
		}
		else Data->Data.Set("",0,true);
	}
	int i=parseSentence();
	if (i<0) return i;
	if (moveFirst()) return 0;
	return 1;
}

int HTQLParser::parseSentence(){
	int End=false;
	int Dot=true;
	int Reverse=false;
	int err=0;
	while(!End){
		if (Syntax.Type == QLSyntax::synQL_DASH){
			Reverse = true;
			Syntax.match();
		}else{
			Reverse = false;
		}

		switch (Syntax.Type){
		case QLSyntax::synQL_DASH:
		case QLSyntax::synQL_DOT:
			break;
		case QLSyntax::synQL_LTAG:
		case QLSyntax::synQL_COLON:
			err=parseTagSelection();
			QueryPatternNum++;
			break;
		case QLSyntax::synQL_LBRACKET:
			err=parseSchema();
			QueryPatternNum++;
			break;
		case QLSyntax::synQL_LBRACE:
			Syntax.match();
			err=parseSentence();
			if (!Syntax.match(QLSyntax::synQL_RBRACE)) return htqlSYNTAXERR;
			break;
		case QLSyntax::synQL_SLASH: 
			if (Dot) err=parseTagSelection();
			else err=parseSchemaReduction();
			QueryPatternNum++;
			break;
		case QLSyntax::synQL_STAR:
			err=parseSchemaReduction();
			QueryPatternNum++;
			break;
		case QLSyntax::synQL_REF:
			err=parseFunctions();
			QueryPatternNum++;
			break;
		case QLSyntax::synQL_STRING:
			err=parseString();
			QueryPatternNum++;
			break;
		default:
			End=true;
			break;
		}
		if (err<0) break;
		if (Reverse){
			reverseResultTags();
		}

		if (Syntax.Type == QLSyntax::synQL_DOT){
			Dot = true;
			Syntax.match();
		}else{
			Dot = false;
		}
	}
	return 0;
}
//======= How the results are stored? 
//
//     SSSSSS---------FFFDDD--------FFFDDD   
//        |
//     FFFFFF---------RRRRRR--------DDDDDD
//        |             |
//        |           SSSS--FFDD--FFDD
//        |             |
//        |           FFFF--DDDD--DDDD  
//        |
//        |
//     FFFFFF---------DDDDDD--------DDDDDD   
//
// ==> SSSS: itemSHEMA_DEF -- schema definition
// ==> DDDD: itemSET -- field data 
// ==> FFFF: itemSCHEMA -- point to a record
// ==> RRRR: itemSCHEMA_REF -- point to a relation
// ==> FFDD: itemSET -- field name

int HTQLParser::switchResults(){
	HTQLItem** res=0;
//	if (IsMidResult && Data){
	if (Data){
		for (res=&Data; (*res); res=& (*res)->NextItem );
		(*res)=MidResults;
		MidResults=Data;
//	}else if (Data){
//		delete Data;
//		Data=0;
	}
	Data = Results;
	IsMidResult=true;
	Results=0;
	return 0;
}

int HTQLParser::parseString(){
	if (Data) switchResults();
	if (Data) switchResults();
	Data=new HTQLItem(0); //string has no parent? 
	Data->Data.Set(Syntax.Sentence + Syntax.Start+1, Syntax.StartLen-2, true);
	Syntax.match();
	return 0;
}

int HTQLParser::parseFunctions(){
	while (Syntax.Type == QLSyntax::synQL_REF){
		Syntax.match();
		if (Syntax.Type == QLSyntax::synQL_WORD){
			//parse function name;
			ReferData fun_name;
			fun_name.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
			Syntax.match();

			//parse function parameters;
			resetHtqlFunctionParameters();

			ReferLink** fun_para = &FunctionParameters;
			if (Syntax.Type == QLSyntax::synQL_LBRACE){
				Syntax.match();
				while (Syntax.Type != QLSyntax::synQL_RBRACE && Syntax.Type != QLSyntax::synQL_END && Syntax.Type != QLSyntax::synQL_UNKNOW){
					//for every parameter
					tExprCalc* expr = new tExprCalc;
					expr->setExpression(Syntax.Sentence + Syntax.Start, 0);
					expr->parse(synEXP_RIGHTBRACE);

					Syntax.Next = Syntax.Start+expr->ExprSentence->Start;
					Syntax.NextLen = 0;
					Syntax.match();
					Syntax.match();

					*fun_para = new ReferLink;
					(*fun_para)->Data = (long) expr;
					fun_para = &(*fun_para)->Next;

					/*
					if (Syntax.Type == QLSyntax::synQL_STRING){
						//the parameter is a string value
						*fun_para = new ReferLink;
						(*fun_para)->Value.Set(Syntax.Sentence+Syntax.Start+1, Syntax.StartLen-2, true);
						fun_para = &(*fun_para)->Next;
					}else if (Syntax.Type == QLSyntax::synQL_NUMBER){
						//the parameter is a number value
						*fun_para = new ReferLink;
						(*fun_para)->Value.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
						fun_para = &(*fun_para)->Next;
					}else if (Syntax.Type == QLSyntax::synQL_WORD){
						// the parameter is a word of fieldname;
						*fun_para = new ReferLink;
						(*fun_para)->Name.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
						if (Data->ItemType == HTQLItem::itemSCHEMA_DEF){
							HTQLItem* def=Data->NextField;
							int field_index=1;
							while (def && def->FieldName.Cmp( (*fun_para)->Name.P, (*fun_para)->Name.L, false) ) {
								def = def->NextField;
								field_index ++;
							}
							if (def) {
								(*fun_para)->Data = field_index;
							}
						}
						fun_para = &(*fun_para)->Next;
					}else{
						return htqlSYNTAXERR;
					}
					*/

					if (Syntax.Type == QLSyntax::synQL_COMMA || Syntax.Type == QLSyntax::synQL_SEMICOLON){
						Syntax.match();
					}
				}
				Syntax.match();
			}

			CurrentFunction = getHtqlFunction(&fun_name);
			HTQLItem* first_item = Data;
			while (first_item && first_item->ItemType == HTQLItem::itemSCHEMA_DEF){
				first_item = first_item->NextItem;
			}

			if (CurrentFunction && CurrentFunction->FunPrepare){
				calculateFunctionParameters(Data, first_item);
				executeFunction((void*)CurrentFunction->FunPrepare, Data);
			}

			if (CurrentFunction && CurrentFunction->FunItem) {
				applyFunctionItem(Data,  CurrentFunction);
			}

			if (CurrentFunction && CurrentFunction->FunComplete){
				calculateFunctionParameters(Data, first_item);
				executeFunction((void*)CurrentFunction->FunComplete, Data);
			}

		}
	}
	return 0;
}

int HTQLParser::addHtqlFunction(char* fun_name, void* fun, char* description, HTQLFunction** htql_fun){
	ReferLink* fun_link=(ReferLink*) HtqlFunctions.findName(fun_name); 
	int len=strlen(fun_name);

	int ret=0;
	if (!fun_link){
		fun_link = (ReferLink*) HtqlFunctions.add(fun_name, 0, 0); 
		ret = 1;
	}else{
		delete (HTQLFunction*) fun_link->Data;
		fun_link->Data = 0;
	}
	HTQLFunction* qlfun = new HTQLFunction;
	qlfun->Name=fun_name;
	qlfun->FunItem = (int (*) (char*, void*, ReferData* ) )fun;
	qlfun->Description.Set(description, strlen(description), true);
	fun_link->Data = (long) qlfun;

	if (htql_fun){
		*htql_fun=qlfun;
	}

	return ret;
}
HTQLFunction* HTQLParser::getHtqlFunction(ReferData* fun_name){
	ReferLink* fun_link= HtqlFunctions.findName(fun_name);
		
	if (fun_link ){
		return (HTQLFunction*) fun_link->Data;
	}else{
		ReferLink* link=ExprContext.Context->findFunctionName(fun_name);
		if (link){
			HTQLFunction* qlfun = new HTQLFunction;
			qlfun->Name=fun_name->P;
			qlfun->FunItem = functionExprFunction;
			qlfun->Description.Set("Default expression function", strlen("Default expression function"), true);

			fun_link = (ReferLink*) HtqlFunctions.add(fun_name, 0, (long) qlfun); 
			return qlfun;
		}
	}

	return 0;
}

int HTQLParser::resetHtqlFunctions(){
	ReferLink* qlfun= HtqlFunctions.getReferLinkHead();
	while (qlfun){
		if (qlfun->Data){
			HTQLFunction* fun = (HTQLFunction*) qlfun->Data;
			if (fun->FunFinalRelease){
				(*fun->FunFinalRelease)(fun);
			}
			delete fun;
			qlfun->Data = 0;
		}
		qlfun = qlfun->Next;
	}
	HtqlFunctions.empty(); 
	return 0;
}

int HTQLParser::resetHtqlFunctionParameters(){
	for (ReferLink* r=FunctionParameters; r; r=r->Next){
		if (r->Data){
			delete (tExprCalc*) r->Data;
			r->Data = 0;
		}
	}
	if (FunctionParameters) {
		delete FunctionParameters;
		FunctionParameters=0;
	}
	return 0;
}

int HTQLParser::calculateFunctionParameters(HTQLItem* schema_def, HTQLItem* schema_data){
	tExprCalc* expr;
	for (ReferLink* r=FunctionParameters; r; r=r->Next){
		if (r->Data){
			r->Value.reset();
			expr = (tExprCalc*) r->Data;
			setExprParam(schema_def, schema_data, expr);
			if (expr->calculate()>=0){
				char* p=expr->getString();
				if (p){
					r->Value.Set(p, strlen(p), false);
				}
			}
		}
	}
	return 0;
}


int HTQLParser::functionHelp(char* data, void* call_from, ReferData* tx){
	tx->reset();

	HTQLParser* call = (HTQLParser*) call_from;
	tx->Cat("<functions>\n", strlen("<functions>\n"));
	ReferLink* fun_link= call->HtqlFunctions.getReferLinkHead();
	while (fun_link){
		tx->Cat("<function name='", strlen("<function name='"));
		tx->Cat(fun_link->Name.P, fun_link->Name.L);
		tx->Cat("'>", strlen("'>") );
		if (fun_link->Data){
			tx->Cat(((HTQLFunction*)fun_link->Data)->Description.P, ((HTQLFunction*)fun_link->Data)->Description.L);
		}
		tx->Cat("</function>\n", strlen("</function>\n") );
		fun_link = fun_link->Next;
	}
	tx->Cat("</functions>\n", strlen("</functions>\n"));
	return 1;
}

int HTQLParser::functionAbout(char* data, void* call_from, ReferData* tx){
	*tx="<About Software='HTQL'>\n"
	   "<Author Name='Liangyou Chen'/>\n"
	   "<Version Release='";
	*tx+=__DATE__;
#ifdef VER_EVALUATION
	*tx+="' Expired='";
	time_t eval_t0=0;
	tStrOp::DateToLong(VER_EVALUATION, "YYYY/MM/DD HH24:MM:SS", &eval_t0);
	eval_t0 += 3600L*24L*30L*VER_EVALUATION_MONTHS;
	char buf[128]="";
	tStrOp::DateToChar(eval_t0, "Mon DD YYYY", buf);
	*tx+=buf;
#endif
#ifdef _DEBUG
	*tx+="' Debug='ON";
#endif
	*tx+="'/>\n</About>";
	return 1;
}

int HTQLParser::applyFunctionField(HTQLItem* data,void* fun){
	HTQLItem* item=0;

//	for (item=data; item; item=item->NextField){
		applyFunctionItem(data, fun);
//	}
	return 0;
}

int HTQLParser::applyFunctionItem(HTQLItem* data,void* fun){
	HTQLItem* item=0;
	HTQLFunction* qlfun=(HTQLFunction*) fun;
	//char* p;

	// calculateFunctionParameters's parameter needs to be data or Data ????

	if (qlfun->FunItemSetPrepare){
		if (FunctionParameters) 
			//calculateFunctionParameters(data, data);
			calculateFunctionParameters(Data, data);
		executeFunction((void*)qlfun->FunItemSetPrepare, data);
	}

	for (item = data; item; item=item->NextItem){
		if (item->ItemType == HTQLItem::itemSET && qlfun->FunItem){
			if (item->Data.P){
				item->Data.Seperate();

				if (FunctionParameters) 
					calculateFunctionParameters(Data, item);
				executeFunction((void*)qlfun->FunItem, item);

				/*
				//if the item is still a SET item
				if (item->ItemType == HTQLItem::itemSET && p){
					if (p!=item->Data.P){
						item->Data.Set(p, strlen(p), true);
					}else{
						item->Data.L = strlen(p);
					}
					item->S.Set(item->Data.P,0,false);
					item->E.Set(item->Data.P+item->Data.L, 0, false);
				}*/
			}

		}else if(item->ItemType == HTQLItem::itemSCHEMA ){

			if (qlfun->FunSchemPrepare){
				if (FunctionParameters) 
					calculateFunctionParameters(Data, item);
				executeFunction((void*)qlfun->FunSchemPrepare, data);
			}

			if (FunctionParameters && item->NextField) {
				calculateFunctionParameters(Data, item);

				HTQLItem* setitem = item->NextField->getSetItem();
				if (!setitem) setitem = item->NextField;
				executeFunction((void*)qlfun->FunItem, setitem);
				
			}else{
				applyFunctionField(item->NextField, fun);
			}

			if (qlfun->FunSchemComplete){
				if (FunctionParameters) 
					calculateFunctionParameters(Data, item);
				executeFunction((void*)qlfun->FunSchemComplete, data);
			}
		}

	}

	if (qlfun->FunItemSetComplete){
		if (FunctionParameters) 
			calculateFunctionParameters(Data, data);
		executeFunction((void*)qlfun->FunItemSetComplete, data);
	}
	return 0;
}

char* HTQLParser::executeFunction(void* fun, HTQLItem* data){
	int ret=0;
	int (*fun1) (char*, void*, ReferData* );
	fun1=(int (*) (char*, void*, ReferData* ) ) fun;

	FunctionCurrentItem = data;
	if (!data) return 0;

	ReferData result;
	if (fun1) ret=(*fun1)(data->Data.P, this, &result);
	if (ret>0) { //ret<0: error; ret==0:no change
		data->Data.Set(result.P, result.L, false);
		if (result.P) data->Data.setToFree(true);
		result.setToFree(false);

		if (data->ItemType == HTQLItem::itemSET){
			data->S.Set(data->Data.P,0,false);
			data->E.Set(data->Data.P+data->Data.L, 0, false);
		}
	}

	return data->Data.P;
}

int HTQLParser::functionTX(char* data, void* call_from, ReferData* tx){
	*tx="";
	if (!data) return 0;
	HTQLParser* call = (HTQLParser* )call_from;
	long space_insert=0;
	long dot_insert=0;
	ReferData keep_tags; 
	if (call && call->FunctionParameters){
		readHtqlParameterLong(call->FunctionParameters, 1, &space_insert);
		readHtqlParameterLong(call->FunctionParameters, 2, &dot_insert);
		readHtqlParameter(call->FunctionParameters, 3, &keep_tags);
	}

	const char* paragraph[]={"</p>", "</h1>", "</h2>", "</h3>", "</li>", "</tr>", "<br>", 0};	
	const char* ignore[]={"<script>", "<style>", 0};
	const char* ignore_end[]={"</script>", "</style>", 0};
	const char* sentence_symbols=".!?;,:-";
	ReferLinkHeap keep_tags_set; 
	if (keep_tags.L){
		tStrOp::splitString(keep_tags.P, "|", &keep_tags_set);
	}
	ReferData text;
	HTQLTagDataSyntax syn;
	syn.setSentence(data);
	int sp=0;
	int to_skip=0;
	int i;
	long first_pos=-1;
	char* p;
	while (syn.Type!= HTQLTagDataSyntax::synQL_END){
		if (!to_skip && (syn.Type == HTQLTagDataSyntax::synQL_DATA 
						|| TagOperation::isTags(syn.Sentence+syn.Start, &keep_tags_set)>=0
						|| TagOperation::isEndTags(syn.Sentence+syn.Start, &keep_tags_set)>=0
				) ){
			if (sp==2 && !tStrOp::isSpace(syn.Sentence[syn.Start])) text.Cat(" ",1);
			text.Cat(syn.Sentence+syn.Start, syn.StartLen);
			sp=1;
			if (first_pos<0) first_pos=syn.Start;
		}else{
			//at least one space after the tag
			if (sp==1 && ( (syn.Start+syn.StartLen<syn.Data.L && tStrOp::isSpace(syn.Sentence[syn.Start+syn.StartLen+1])) || space_insert) ) sp=2;

			if (syn.Type==HTQLTagDataSyntax::synQL_END_TAG && text.L && TagOperation::isTags(syn.Sentence+syn.Start, paragraph)>=0){
				for (p=text.P+text.L-1; p>=text.P && tStrOp::isSpace(*p); p--);
				if (dot_insert && p>=text.P && !strchr(sentence_symbols, *p)){
					text+=".";
				}
			}else if (syn.Type==HTQLTagDataSyntax::synQL_START_TAG && (i=TagOperation::isTags(syn.Sentence+syn.Start, ignore))>=0 ){
				to_skip=i+1;
			}else if (to_skip>0 && syn.Type==HTQLTagDataSyntax::synQL_END_TAG && TagOperation::isTags(syn.Sentence+syn.Start, ignore_end)==to_skip-1 ){
				to_skip=0;
			}else if (!to_skip && TagOperation::isTags(syn.Sentence+syn.Start, ignore)>=0 ){
				
			}
		}
		syn.match();
	}
	if (first_pos>0 && call && call->FunctionCurrentItem) call->FunctionCurrentItem->SourceOffset+=first_pos;

	return functionDecodeHtml(text.P, call_from, tx);
}

int HTQLParser::functionTxStr(char* data, void* call_from, ReferData* tx){
	*tx="";

	ReferData text;
	functionTX(data, call_from, &text);
	char* p=text.P;

	long max_len=0;
	HTQLParser* call = (HTQLParser* )call_from;
	if (call){
		readHtqlParameterLong(call->FunctionParameters, 1, &max_len);
	}

	long first_pos=tStrOp::trimCRLFSpace(p, tx, max_len); 

	if (first_pos>0 && call) call->FunctionCurrentItem->SourceOffset+=first_pos;

	return 1;
}

int HTQLParser::readHtqlParameterLong(ReferLink* parameters, int index1, long* val){
	ReferLink* para = parameters;
	int i=1;
	while (i<index1 && para){
		para = para->Next;
		i++;
	}
	if (i==index1 && para && !para->Value.isNULL() ){
		sscanf(para->Value.P, "%ld", val);
	}
	return 0;
}
ReferLink* HTQLParser::readHtqlParameter(ReferLink* parameters, int index1, ReferData* val){
	ReferLink* para = parameters;
	int i=1;
	while (i<index1 && para){
		para = para->Next;
		i++;
	}
	if (i==index1 && para && !para->Value.isNULL() ){
		if (val) val->Set(para->Value.P, para->Value.L, false);
	}
	return para;
}

int HTQLParser::functionDecodeHtml(char*p, void* call_from, ReferData* tx){
	if (p) *tx=p; else *tx="";
	tStrOp::decodeHtml(tx->P);
	tx->L=strlen(tx->P);

	return 1;
}

int HTQLParser::toHtmlPrintable(char*p, void* call_from, ReferData* tx){
	tx->reset();

	if (!p) return 0;
	tx->P=tStrOp::encodeHtml(p, &tx->Type, &tx->Size, &tx->L);
	/*char* p1, *p2;
	p1=tStrOp::replaceMalloc(p, "<", "&lt;");
	p2=tStrOp::replaceMalloc(p1, ">", "&gt;");
	tx->Set(p2, strlen(p2), false);
	tx->setToFree(true);
	free(p1);*/
	return 1;
}
int HTQLParser::functionEncodeUrl(char*p, void* call_from, ReferData* tx){
	HtmlBuffer buf;
	*tx=buf.encodeUrl(p);
	return 1;
}
int HTQLParser::functionDecodeUrl(char*p, void* call_from, ReferData* tx){
	HtmlBuffer buf;
	*tx=buf.decodeUrl(p);
	return 1;
}

int HTQLParser::functionGetUrl(char* tag, void* call_from, ReferData* tx){
	//does not translate to absolute url, use &get_url &url to get it.
	tx->reset(); //set to NULL

	HTQLParser* call = (HTQLParser* ) call_from;
	if (!call->FunctionCurrentItem->S.P) return 1;
	ReferData attr;
	if (TagOperation::isTag(call->FunctionCurrentItem->S.P, "A") ||
		TagOperation::isTag(call->FunctionCurrentItem->S.P, "base")){
		attr="href";
	}else if (TagOperation::isTag(call->FunctionCurrentItem->S.P, "img") ||
		TagOperation::isTag(call->FunctionCurrentItem->S.P, "image") ||
		TagOperation::isTag(call->FunctionCurrentItem->S.P, "frame")){
		attr="src";
	}else{
		attr="href";
	}
	unsigned int len=0;
	char* p=TagOperation::targetAttribute(call->FunctionCurrentItem->S.P, attr.P, &len);
	if (!p) return 1;
	p=TagOperation::targetValue(p, &len);
	if (!p) {
		*tx=""; 
		return 1;
	}

	tx->Set(p, len, true);
	call->FunctionCurrentItem->SourceOffset+=p-call->FunctionCurrentItem->S.P;
	return 1;
}

int HTQLParser::functionUrl(char* url, void* call_from, ReferData* tx){
	tx->reset(); 
	if (!url) return 1; 
	HTQLParser* call = (HTQLParser* ) call_from;
	if (!call->BaseUrl.P){
		HTQLParser base_parser;
		base_parser.setData(call->SourceData.P, call->SourceData.L, false);
		base_parser.setSentence("<base>1:href", 12, true);
		base_parser.parse();
		char* p=base_parser.getField(1);
		if (p && *p){
			call->BaseUrl.Set(p, strlen(p), true);
		}else{
			if (call->CurrentSourceUrl.P){
				call->BaseUrl.Set(call->CurrentSourceUrl.P, call->CurrentSourceUrl.L, true);
			}else{
				call->BaseUrl.Set("",0,true);
			}
		}
	}
	ReferData url1; 
	url1.Set(url, strlen(url), true); 
	tStrOp::decodeHtml(&url1); 
	return call->mergeUrl(call->BaseUrl.P, url1.P, tx);
}

int HTQLParser::functionUrlAllOld(char* p, void* call_from, ReferData* tx){
	HTQLParser* call = (HTQLParser* ) call_from;
	if (!call->CurrentSourceUrl.L) return 0;	
	HTQL ql, qlupdate;
	ReferLinkHeap updated_urls;

	ReferData text;
	functionUrl("", call, &text); //set up call->BaseUrl;

	ql.setSourceData(p, strlen(p), false);
	ql.setSourceUrl(call->BaseUrl.P, call->BaseUrl.L);

	qlupdate.setSourceData(p, strlen(p), false);
	qlupdate.setSourceUrl(call->BaseUrl.P, call->BaseUrl.L);

	char* queries[][2]={ 
		{"<a>{url=:href; full=:href&url}", "<a (href=a)>:href &replace(b)"},
		{"<form>{url=:action; full=:action&url}", "<form (action=a)>:action &replace(b)"},
		{"<img>{url=:src; full=:src&url}", "<img (src=a)>:src &replace(b)"},
		{"<image>{url=:src; full=:src&url}", "<image (src=a)>:src &replace(b)"},
		{"<area>{url=:href; full=:href&url}", "<area (href=a)>:href &replace(b)"},
		{"<frame>{url=:src; full=:src&url}", "<frame (src=a)>:src &replace(b)"},
		{"<embed>{url=:src; full=:src&url}", "<embed (src=a)>:src &replace(b)"},
		{"<object>{url=:codeBase; full=:codeBase&url}", "<object (codeBase=a)>:codeBase &replace(b)"},
		{0,0}
	};


	ReferData url;
	for (int i=0; queries[i][0]; i++){
		updated_urls.reset();
		ql.setQuery(queries[i][0]);
		while (!ql.isEOF()){
			char* p1=ql.getValue(1);
			char* p2=ql.getValue(2);
			if (!p1 || !p2) {
				ql.moveNext();
				continue;
			}

			url.Set(p1, strlen(p1), false);
			if (!updated_urls.findName(&url)){
				qlupdate.setGlobalVariable("a", p1);
				qlupdate.setGlobalVariable("b", p2);
				qlupdate.setQuery(queries[i][1]);
			}
			ql.moveNext();
		}
	}
	*tx = qlupdate.getSourceData();
	return 1;
}
int HTQLParser::functionUrlAll(char* p, void* call_from, ReferData* tx){
	HTQLParser* call = (HTQLParser* ) call_from;
	if (!call->CurrentSourceUrl.L) return 0;	
	HTQL ql;
	ReferLinkHeap update_urls;
	update_urls.setSortOrder(SORT_ORDER_NUM_INC);
	update_urls.setDuplication(false);

	ReferData text;
	functionUrl("", call, &text); //set up call->BaseUrl;

	ql.setSourceData(p, strlen(p), false);
	ql.setSourceUrl(call->BaseUrl.P, call->BaseUrl.L);

	char* queries[]={ 
		"<a>{url=:href; full=:href&url}", 
		"<form>{url=:action; full=:action&url}", 
		"<img>{url=:src; full=:src&url}", 
		"<image>{url=:src; full=:src&url}", 
		"<area>{url=:href; full=:href&url}", 
		"<frame>{url=:src; full=:src&url}", 
		"<iframe>{url=:src; full=:src&url}", 
		"<link (href is not null )>{url=:href; full=:href&url}", 
		//"<link (type like 'text/css' or rel like 'stylesheet' )>{url=:href; full=:href&url}", 
		"<script (src is not null)>{url=:src; full=:src&url}", 
		"<embed>{url=:src; full=:src&url}", 
		"<object>{url=:codeBase; full=:codeBase&url}",
		0
	};


	ReferData url;
	ReferLink* link;
	long pos=0, offset=0;
	for (int i=0; queries[i]; i++){
		ql.setQuery(queries[i]);
		while (!ql.isEOF()){
			char* p1=ql.getValue(1);
			char* p2=ql.getValue(2);
			pos=0; offset=0;
			ql.getFieldOffset(1, &pos, &offset);
			if (!p1 || !p2 || pos<0 || offset<0) {
				ql.moveNext();
				continue;
			}

			url.Set(p2, strlen(p2), false);
			link=update_urls.add(&url, 0, pos);
			if (link){
				link->Value.L = offset; 
			}
			ql.moveNext();
		}
	}
	long add_len=0; 
	for (link=(ReferLink*) update_urls.moveFirst(); link; link=(ReferLink*) update_urls.moveNext()){
		add_len+=link->Name.L; 
	}
	tx->Malloc(strlen(p)+add_len);
 
	long pos0 = 0;
	for (link=(ReferLink*) update_urls.moveFirst(); link; link=(ReferLink*) update_urls.moveNext()){
		pos = link->Data; 
		offset = link->Value.L;
		tx->Cat(p+pos0, pos-pos0);
		tx->Cat(link->Name.P, link->Name.L);
		pos0 = pos+offset;
	}
	tx->Cat(p+pos0, strlen(p+pos0));

	return 1;
}

int HTQLParser::functionSkipHeader(char*p, void* call_from, ReferData* tx){
	if (!p) return 0;
	HTQLParser* call = (HTQLParser* )call_from;
	char* p1 = strstr(p, "\r\n\r\n");
	if (p1) {
		*tx = p1+4;
		call->FunctionCurrentItem->SourceOffset+=p1-p+4;
		return 1;
	}

	return 0;
}

int HTQLParser::functionRemoveHeader(char*p, void* call_from, ReferData* tx){
	HTQLParser* call = (HTQLParser* )call_from;

	if (!call->SourceData.P) return 0;

	ReferData text;
	char* p1 = strstr(call->SourceData.P, "\r\n\r\n");
	if (p1) {
		text.Set(p1+4, call->SourceData.L - (p1-call->SourceData.P) - 4, true);
		call->SourceData.Set(text.P, text.L, false);
		call->SourceData.setToFree(true);
		text.setToFree(false);

		call->resetData();
	}
	return 0;
}

int HTQLParser::functionSetSource(char*p, void* call_from, ReferData* tx){
	HTQLParser* call = (HTQLParser* )call_from;

	ReferLink* para =call->FunctionParameters;
	if (!para || para->Value.isNULL()){
		call->SourceData.Set(p, p?strlen(p):0, true);
	}else{
		call->SourceData.Set(para->Value.P, para->Value.L, true);
	}
	call->resetData();
	return 0;
}
int HTQLParser::functionGetCsvField(char*p, void* call_from, ReferData* tx){
	*tx = "";
	HTQLParser* call = (HTQLParser* )call_from;

	ReferLink* para =call->FunctionParameters;
	ReferData sep; sep=",";
	if (para && para->Value.L) sep=para->Value; 
	int index=1;
	if (para && para->Next && para->Next->Value.L) index=para->Next->Value.getLong(); 

	int is_reverse=false;
	if (index<0) {
		is_reverse=true;
		index=-index;
	}
	ReferLinkHeap fields; 
	fields.setSortOrder(SORT_ORDER_NUM_INC);
	tStrOp::splitCsvString(p, sep.P, &fields);
	int count=0;
	if (index==0){
		ReferData line_sep; line_sep="\n"; 
		if (para && para->Next && para->Next->Value.L) line_sep=para->Next->Value;
		for (ReferLink* link=(ReferLink*) fields.moveFirst(); link; link=(ReferLink*) fields.moveNext()){
			//*tx+=link->Value;
			*tx+=link->Name; 
			//*tx+=link->Value;
			*tx+=line_sep; 
		}
	}else if (is_reverse){
		for (ReferLink* link=(ReferLink*) fields.moveLast(); link; link=(ReferLink*) fields.movePrevious()){
			if (++count==index) { //found
				*tx=link->Name; 
				call->FunctionCurrentItem->SourceOffset=call->FunctionCurrentItem->SourceOffset+link->Data; 
			}
		}
	}else{
		for (ReferLink* link=(ReferLink*) fields.moveFirst(); link; link=(ReferLink*) fields.moveNext()){
			if (++count==index) { //found
				*tx=link->Name; 
				call->FunctionCurrentItem->SourceOffset=call->FunctionCurrentItem->SourceOffset+link->Data; 
			}
		}
	}
	return 1;
}
int HTQLParser::functionGetDate(char*p, void* call_from, ReferData* tx){
	*tx = "";

	HTQLParser* call = (HTQLParser* )call_from;
	int index=1;
	ReferLink* para = call->FunctionParameters;
	if (para && para->Value.L) index=para->Value.getLong(); 
	if (index==0) return 0;
	ReferData option;
	if (para && para->Next && para->Next->Value.L ){
		option=para->Next->Value.P; // "usedays"
	}

	int is_reverse=false;
	if (index<0) {
		is_reverse=true;
		index=-index;
	}
	ReferLinkHeap dates; 
	dates.setSortOrder(SORT_ORDER_NUM_INC);
	if (p){
		TagOperation::searchHtmlDateStr(p, strlen(p), &dates, option.Cmp("exact", 5, false));
	}
	int count=0;
	if (is_reverse){
		for (ReferLink* link=(ReferLink*) dates.moveLast(); link; link=(ReferLink*) dates.movePrevious()){
			if (++count==index) { //found
				*tx=link->Value; 
				call->FunctionCurrentItem->SourceOffset=call->FunctionCurrentItem->SourceOffset+link->Data; 
			}
		}
	}else{
		for (ReferLink* link=(ReferLink*) dates.moveFirst(); link; link=(ReferLink*) dates.moveNext()){
			if (++count==index) { //found
				*tx=link->Value; 
				call->FunctionCurrentItem->SourceOffset=call->FunctionCurrentItem->SourceOffset+link->Data; 
			}
		}
	}
	return 1;
}

int HTQLParser::functionGetNumber(char*p, void* call_from, ReferData* tx){
	*tx = "";

	HTQLParser* call = (HTQLParser* )call_from;
	int index=1;
	ReferLink* para =call->FunctionParameters;
	if (para && para->Value.L) index=para->Value.getLong(); 
	if (index==0) return 0;

	if (tStrOp::getNumber(p, index, tx)) {
		call->FunctionCurrentItem->SourceOffset=call->FunctionCurrentItem->SourceOffset+(tx->P-p); 
		tx->Seperate(); 
	}
	return 1;
}
int HTQLParser::functionExprFunction(char*p, void* call_from, ReferData* tx){//any other functions
	HTQLParser* call = (HTQLParser* )call_from;
	ReferData sentence; 
	tExprCalc calc; 

	//compose expression, and set calc parameters
	sentence=call->CurrentFunction->Name; 
	sentence+="("; 
	int i=0; 
	ReferLink* link; 
	char paraname[256]; 
	sprintf(paraname, "para%d", i++); 
	sentence+=paraname; 
	calc.setVariable(paraname, p, p?strlen(p):0, false); 
	for (link=call->FunctionParameters; link; link=link->Next){
		sprintf(paraname, "para%d", i++); 
		sentence+=", "; 
		sentence+=paraname; 
		calc.setVariable(paraname, link->Value.P, link->Value.L, false); 
	}
	sentence+=")";

	calc.setExpression(sentence.P, &sentence.L);
	calc.parse(synEXP_RIGHTBRACE);
	calc.calculate(); 
	*tx=calc.getString();
	return 1;
}
int HTQLParser::functionTupleCount(char*p, void* call_from, ReferData* tx){//&tuple_count()
	HTQLParser* call = (HTQLParser* )call_from;
	long tuple_count=call->getTuplesCount();
	long offset=call->Data->SourceOffset;
	call->resetData(); 
	call->Data->Data.setLong(tuple_count); 
	call->Data->SourceOffset=offset;
	call->Data->S.Set(call->Data->S.P, 0, false); 
	call->Data->E.Set(call->Data->S.P+call->Data->S.L, 0, false); 
	return 0;
}
int HTQLParser::addDllFunctions(ReferLink* loadedfuncs, long para){
	for (ReferLink* link=loadedfuncs; link; link=link->Next){
		if (link->Name.L && link->Value.P){
			link->Data=para; 
			addHtqlFunction(link->Name.P, (void*) functionDllFunction, "DLL function");
		}
	}

	return 0;
}
int HTQLParser::functionDllFunction(char*p, void* call_from, ReferData* tx){
	*tx = "";
	HTQLParser* call = (HTQLParser*) call_from;
	ReferLink* para = call->FunctionParameters;
	int argc=1;
	ReferLink* link;
	for (link=para; link; link=link->Next){
		argc++; 
	}
	char** argv=(char**) malloc(sizeof(char*)*(argc+1));
	argv[0]=p;
	int i=1;
	for (link=para; link; link=link->Next){
		argv[i++]=link->Value.P; 
	}
	argv[argc]=0; 

	int (*dllfunc)(int, char**, char**, long*);
	dllfunc=(int (*)(int, char**, char**, long*)) ((ReferLink*) p)->Value.P;
	char* results=0; 
	long len=0; 
	dllfunc(argc, argv, &results, &len);
	if (results){
		tx->Set(results, len, true); 
#ifdef _WINDOWS
		//The DLL function needs to use GlobalAlloc() function to allocate memoe
		GlobalFree(results);
#else
		free(results);
#endif
	}
	if (argv) free(argv); 
	return 1;
}

int HTQLParser::functionAfter(char*p, void* call_from, ReferData* tx){
	*tx = "";

	HTQLParser* call = (HTQLParser* )call_from;
	int index=1;
	ReferData pattern;
	ReferLink* para =call->FunctionParameters;
	if (!para || para->Value.isNULL()) return 0;
	pattern = para->Value.P;
	if (pattern.isNULL() ) return 0;
	HTQLTagSelection::parseSpecial(&pattern);

	if (para->Next && !para->Next->Value.isNULL() ){
		sscanf(para->Next->Value.P, "%d", &index);
	}

	char* p1=p;
	for (int i=0; i<index; i++){
		if (!p1) break;
		p1 = strstr(p1, pattern.P);
		if (p1) p1+= pattern.L;
	}
	if (p1) *tx = p1;

	return 1;
}
int HTQLParser::functionBefore(char*p, void* call_from, ReferData* tx){
	*tx = "";

	HTQLParser* call = (HTQLParser* )call_from;
	int index=1;
	ReferData pattern;
	ReferLink* para =call->FunctionParameters;
	if (!para || para->Value.isNULL()) return 0;
	pattern = para->Value.P;
	if (pattern.isNULL() ) return 0;
	HTQLTagSelection::parseSpecial(&pattern);

	if (para->Next && !para->Next->Value.isNULL() ){
		sscanf(para->Next->Value.P, "%d", &index);
	}

	char* p1=p;
	for (int i=0; i<index; i++){
		if (!p1) break;
		p1 = strstr(p1, pattern.P);
		if (p1) p1+= pattern.L;
	}
	if (p1>p) p1 -= pattern.L;
	if (p1) tx->Set(p, p1-p, true);

	return 1;
}
int HTQLParser::functionTrim(char*p, void* call_from, ReferData* tx){
	*tx = "";

	HTQLParser* call = (HTQLParser* )call_from;
	ReferData pattern; pattern=" \t\n\r";
	ReferData mark; mark="1010"; //first, middle, last, remove all spaces; 0=no trim; 1=trim; =2; trim CRLF 
					//"121"; trim, no CRLF
	ReferLink* para =call->FunctionParameters;
	if (para && para->Value.L) pattern = para->Value.P;
	if (para && para->Next && para->Next->Value.L>=3 ){
		mark=para->Next->Value.P;
	}
	tStrOp::decodeHtml(pattern.P); pattern.L=strlen(pattern.P); //can use &nbsp; or &#12; 

	tStrOp::trimText(p, tx, pattern.P, mark.P);

	return 1;
}
int HTQLParser::functionCharReplace(char*p, void*call_from, ReferData* tx){
	HTQLParser* call = (HTQLParser* )call_from;
	ReferLink* para =call->FunctionParameters;
	char empty=0;
	char* chars=&empty, *replace=&empty;
	if (para && para->Value.L) chars = para->Value.P;
	if (para && para->Next && para->Next->Value.P){
		replace=para->Next->Value.P;
	}
	tStrOp::replaceChars(p, chars, replace); 

	return 0;
}

int HTQLParser::toUpper(char*p, void* call_from, ReferData* tx){
	char* t=p;
	while (*t){
		*t = toupper(*t);
		t++;
	}
	return 0;
}
int HTQLParser::toLower(char*p, void* call_from, ReferData* tx){
	char* t=p;
	while (*t){
		*t = tolower(*t);
		t++;
	}
	return 0;
}

int HTQLParser::mergeUrl(char* base, char* url, ReferData* merge_result){
	//remove leading spaces in url
	while (url && tStrOp::isSpace(*url)) url++;
	char*protocolend=url;
	while (protocolend && isalpha(*protocolend)) protocolend++;

	if (!base || !base[0]){
		merge_result->Set(url, strlen(url), true);
	}else if (!url){
		merge_result->Set("",0, true);
	}else if (!tStrOp::strNcmp(url,"mailto:",7, false)
			|| !tStrOp::strNcmp(url,"javascript:",11, false)) {
		merge_result->Set(url, strlen(url), true);
	}else if (!tStrOp::strNcmp(protocolend,"://",3, false) || !tStrOp::strNcmp(url,"//",2, false) ) {
		if (protocolend==url){
			char*baseprotocolend=base;
			while (baseprotocolend && isalpha(*baseprotocolend)) baseprotocolend++;

			merge_result->Set(base, baseprotocolend-base, true);

			if (*protocolend != ':') merge_result->Cat(":",1);

			merge_result->Cat(protocolend, strlen(protocolend)); 
		}else{
			merge_result->Set(url, strlen(url), true);
		}
	}else{
		char*baseprotocolend=base;
		while (baseprotocolend && isalpha(*baseprotocolend)) baseprotocolend++;

		char *p=base;
		if (tStrOp::strNcmp(baseprotocolend,"://",3, false)) {
			p=base;
			merge_result->Set("http://", 7, true);
		}else{
			p=baseprotocolend+3;
			merge_result->Set("", 0, true);
		}

		char* p1=strchr(p, '/');
		if (!p1) p1=strchr(p, '\\');
		if (!p1) p1=strchr(p, '?');
		if (!p1) p1=p+strlen(p);

		if (url[0]=='/' || url[0]=='\\' ){
			merge_result->Cat(base, p1-base);
			merge_result->Cat(url, strlen(url) );
		}else if (url[0]=='#'){
			p1=strrchr(p, '#');
			if (!p1) p1=p+strlen(p);

			merge_result->Cat(base, p1-base);
			merge_result->Cat(url, strlen(url) );
		}else if (url[0]=='?'){
			p1=strrchr(p, '?');
			if (!p1) p1=p+strlen(p);

			merge_result->Cat(base, p1-base);
			merge_result->Cat(url, strlen(url) );
		}else if (url[0]=='.'){
			if (url[1]=='.' && (url[2]=='/'||url[2]=='\\')){//parent dir
				char* p2=strrchr(p1, '/');
				if (!p2) p2=strrchr(p1, '\\');
				while (url[0]=='.' && url[1]=='.' && (url[2]=='/'||url[2]=='\\')){
					if (!p2) p2=p1;
					else p2--;
					while (p2>p1 && *p2!='/' && *p2!='\\') p2--;
					if (p2<p1) p2=p1; 
					url=url+3;
				}
				merge_result->Cat(base, p2-base);
				if (merge_result->L && merge_result->P[merge_result->L-1]!='/' && merge_result->P[merge_result->L-1]!='\\') 
					*merge_result+="/"; 
				merge_result->Cat(url, strlen(url));
			}else if (url[1]=='/' || url[1]=='\\'){
				char* p2=strrchr(p1, '/');
				if (!p2) p2=strrchr(p1, '\\');
				if (!p2) p2=p1;
				merge_result->Cat(base, p2-base);
				merge_result->Cat(url+1, strlen(url+1));
			}
		}else{
			p1=strrchr(p,'/'); //base dir
			if (!p1) p1=strrchr(p, '\\');
			if (!p1) p1=strrchr(p, '?');
			if (!p1) p1=p+strlen(p);
			char* p2=strrchr(p, '?');
			if (p2 && p2<p1){
				for (p1=p2; p1[0]!='/' && p1[0]!='\\' && p1>p; p1--);
			}

			if (*p1 == '/' || *p1 == '\\' ){
				merge_result->Cat(base, p1-base+1);
			}else{
				merge_result->Cat(base, p1-base);
				merge_result->Cat("/", 1);
			}
			merge_result->Cat(url, strlen(url) );
		}
	}
	tStrOp::replaceInplace(merge_result->P, "&nbsp;", "+");
	tStrOp::replaceInplace(merge_result->P, "&lt;", "<");
	tStrOp::replaceInplace(merge_result->P, "&gt;", ">");
	tStrOp::replaceInplace(merge_result->P, "&amp;", "&");

	return 1;
}

int HTQLParser::parseSchemaReduction(){
	int dimension=0;
	if (Syntax.Type == QLSyntax::synQL_STAR){
		dimension=Syntax.StartLen;
		Syntax.match();
	}
	if (Syntax.Type != QLSyntax::synQL_SLASH){
		return htqlSYNTAXERR;
	}
	ReferData pat;
	if (Syntax.StartLen>1) pat.Set(Syntax.Sentence+Syntax.Start+1, Syntax.StartLen-2, true);
	HTQLTagSelection::parseSpecial(&pat);
	reduceDimension(Data, dimension, pat.P, pat.L);
	Syntax.match();

	return 0;
}

int	HTQLParser::reduceDimension(HTQLItem* data, int dimension, char* pat, int pat_len){
	if (!data ) return 0;
	HTQLItem* item=0;
	if (dimension == 0){
		reduceDimItem(data, pat, pat_len);
		reduceDimField(data, pat, pat_len);
		if (data->NextField) {// the first itemSCHEMA_DEF become itemSET
			delete data->NextField;
			data->NextField=0;
		}
	}else if (dimension == 1){
		for (item=data; item; item=item->NextItem){
			reduceDimField(item, pat, pat_len);
		}
	}else if (dimension == 2){
		HTQLItem* item1=0;
		for (item=data; item; item=item->NextItem){
			for (item1=item->NextField; item1; item1=item1->NextField){
				reduceDimItem(item1, pat, pat_len);
			}
		}
	}
	return 0;
}

int	HTQLParser::reduceDimField(HTQLItem* data, char* pat, int pat_len){
	HTQLItem* item=0;
	ReferData rd;
	switch (data->ItemType){
	case HTQLItem::itemSET:
	case HTQLItem::itemSCHEMA_REF:
		break;
	case HTQLItem::itemSCHEMA_DEF:
		data->Data.Set("",0, true);
		if (data->NextField) delete data->NextField;
		data->NextField=0;
		break;
	case HTQLItem::itemSCHEMA:
		rd.Set("", 0, true);
		for (item=data->NextField; item; item=item->NextField){
			reduceDimItem(item, pat, pat_len);
			if (item->ItemType == HTQLItem::itemSET){
				if (rd.L) rd.Cat(pat, pat_len);
				rd.Cat(item->Data.P, item->Data.L);
			}
		}
		data->Data.Set(rd.P, rd.L,true);
		data->ItemType = HTQLItem::itemSET;
//		for (item=data->NextField; item->NextField; item=item->NextField);
//		item->NextField=MidResults;
//		MidResults=data->NextField;
		if (data->NextField) delete data->NextField;
		data->NextField=0;
		break;
	default: 
		break;
	}
	data->S.Set(data->Data.P, 0, false);
	data->E.Set(data->Data.P+data->Data.L, 0, false);
	return 0;
}

int	HTQLParser::reduceDimItem(HTQLItem* data, char* pat, int pat_len){
	HTQLItem* item=0;
	ReferData rd;

	for (item=data; item; item=item->NextItem){
		reduceDimField(item, pat, pat_len);
		if (item->ItemType == HTQLItem::itemSET){
			if (rd.L) rd.Cat(pat, pat_len);
			rd.Cat(item->Data.P, item->Data.L);
		}
	}
	data->Data.Set(rd.P,rd.L, true);
	data->ItemType = HTQLItem::itemSET;
	if (!data->isRefNextItem && data->NextItem) delete data->NextItem;
	data->NextItem=0;

	data->S.Set(data->Data.P, 0, false);
	data->E.Set(data->Data.P+data->Data.L, 0, false);
	return 0;
}

int HTQLParser::parseTagSelection(){
	if (Results) switchResults();

	HTQLItem* item;
	HTQLItem** res=&Results;
	HTQLItem* field=0;
	HTQLTagSelection* tag_selection=0;
	int i;

	if (!Data){
		tag_selection = new HTQLTagSelection;
		tag_selection->Options = Options;
		tag_selection->Sentence.Set(Syntax.Sentence+Syntax.Start, Sentence.L-Syntax.Start);
		tag_selection->parseSentence();
		Options = tag_selection->Options;
		Syntax.Next = Syntax.Start;
		Syntax.NextLen = tag_selection->Syntax.Start;
		Syntax.match();
		Syntax.match();
		delete tag_selection;
		return 0;
	}

	int to_copy_last_tag=false; 
	HTQLTag last_tag;   last_tag.copyTag(&LastTag, to_copy_last_tag);
	TagOptions Options0 = Options;

	for (item=Data; item; item=item->NextItem){
		field=(item->getSetItem());

		tag_selection = new HTQLTagSelection;
		tag_selection->ParentItem = item;

		ReferLink* r=new ReferLink;
		r->Next = TagSelections;
		r->Value.Set((char*) tag_selection, 0, false);
		r->Data = htqlTAG_SELECTION;
		TagSelections = r;

		tag_selection->Options = Options0;
		tag_selection->ReferVars = this->GlobalVariables.Next;
		tag_selection->Sentence.Set(Syntax.Sentence+Syntax.Start, Sentence.L-Syntax.Start);
		i=tag_selection->parseSentence();
		Options = tag_selection->Options;
		if (i<0) return i;

		if (!field) continue;

		if (item!=Data) LastTag.copyTag(&last_tag, to_copy_last_tag);
		if ( (tag_selection->Tag.S.P && tag_selection->Tag.TagType == LastTag.TagType &&
				!tag_selection->Tag.S.Cmp(LastTag.S.P, LastTag.S.L, Options.Sen) &&
				!tag_selection->Tag.E.Cmp(LastTag.E.P, LastTag.E.L, Options.Sen) && 
				!Options0.Sep
			)
			|| Options.NoEmbedding
			){//exclude tag in the next selection
			tag_selection->Data.Set(field->Data.P+field->S.L, field->Data.L-field->S.L-field->E.L, false);
			tag_selection->SourceOffset = field->SourceOffset+field->S.L;
		}else{//include tag in the next selection
			tag_selection->Data.Set(field->Data.P, field->Data.L, false);
			tag_selection->SourceOffset = field->SourceOffset;
		}
		tag_selection->LastTag.copyTag(&LastTag, to_copy_last_tag);
		i=tag_selection->parseData();
		if (i<0) return i;
		//tag_selection->parse();
		LastTag.copyTag(&tag_selection->Tag, to_copy_last_tag);

		while (*res) res=& (*res)->NextItem;

		if (tag_selection->Results){
			if (tag_selection->Results->ItemType == HTQLItem::itemSCHEMA_REF){
				(*res) = tag_selection->Results->NextItem;
				tag_selection->Results->NextItem = 0;
			}else{
				(*res) = tag_selection->Results;
				tag_selection->Results = 0;
			}
		}
	}

	if (tag_selection){
		Syntax.Next = Syntax.Start;
		Syntax.NextLen = tag_selection->Syntax.Start;
		Syntax.match();
		Syntax.match();
	}

	//store results to Data;
	switchResults();

	return 0;
}

int HTQLParser::reverseResultTags(){
	HTQLItem** res = &Data;
	long offset=0, offset1, len, len1;
	while (*res){
		offset1 = (*res)->SourceOffset;
		if ((*res)->Data.Type) {
			len1 = 0;
		}else{
			len1 = (*res)->Data.L;
		}

		len = offset1 - offset;
		if (offset <= SourceData.L){
			if (offset+len < SourceData.L){
				(*res)->Data.Set(SourceData.P+offset, len, false);
			}else{
				(*res)->Data.Set(SourceData.P+offset, SourceData.L-offset, false);
			}
			(*res)->SourceOffset = offset;
		}
		
		offset = offset1+len1;
		res = &(*res)->NextItem;
	}
	if (offset < SourceData.L){
		(*res) = new HTQLItem(Data?Data->ParentItem:0); //last item
		(*res)->Data.Set(SourceData.P+offset, SourceData.L-offset, false);
		(*res)->S.Set(SourceData.P+offset, 0, false);
		(*res)->E.Set(SourceData.P+SourceData.L, 0, false);
		(*res)->TagType = Data?Data->TagType:0;
		(*res)->SourceOffset = offset;
	}
	return 0;
}


int HTQLParser::parseSchema(){
	if (Results) switchResults();
	
	int i;
	if (!Syntax.match(QLSyntax::synQL_LBRACKET) ) return htqlSYNTAXERR;
	i= parseSchemaConstruct();
	if (i<0) return i;

	if (Syntax.Type == QLSyntax::synQL_MID){
		Syntax.match();
		i=parseSchemaCondition();
	}

	if (Syntax.Type == QLSyntax::synQL_MID){
		Syntax.match();
		i=parseSchemaTransform();
	}
	
	if (!Syntax.match(QLSyntax::synQL_RBRACKET) ) return htqlSYNTAXERR;

	LastTag.reset();

	return 0;
}
/*
HTQLItem* HTQLParser::getSetItem(HTQLItem* item){
	HTQLItem* item1;
	if (!item || item->ItemType == HTQLItem::itemSET) return item;
	else if (item->ItemType == HTQLItem::itemSCHEMA){
		return getSetItem(item->NextField);
	}else if (item->ItemType == HTQLItem::itemSCHEMA_DEF){
		return 0;
	}else if (item->ItemType == HTQLItem::itemSCHEMA_REF){
		item1=item;
		while (item1 && item1->ItemType != HTQLItem::itemSET){
			if (item1->ItemType==HTQLItem::itemSCHEMA_REF)
				item1 = item1->NextItem;
			else if (item1->ItemType==HTQLItem::itemSCHEMA_DEF) 
				item1=item1->NextItem;
			else if (item1->ItemType==HTQLItem::itemSCHEMA) 
				item1=item1->NextField;
		}
		return item1;
	}
	return 0;
}*/

int HTQLParser::copyRow(HTQLItem* row, HTQLItem** to_addr){
	HTQLItem** res=to_addr;
	for (HTQLItem* field=row; field; field=field->NextField){
		(*res) = new HTQLItem((*to_addr)?(*to_addr):field->ParentItem);
		(*res)->copyTag(field);
		(*res)->FieldName.Set(field->FieldName.P, field->FieldName.L, true);
		(*res)->Data.Set(field->Data.P, field->Data.L, false);
		(*res)->ItemType=field->ItemType;
		if (res!= to_addr) {
			(*res)->NextItem = field->NextItem;
			(*res)->isRefNextItem = true;
		}
		res=&(*res)->NextField;
	}
	return 0;
}

int HTQLParser::copyRowSetItem(HTQLItem* row, HTQLItem** to_addr){
	HTQLItem** res=to_addr;
	HTQLItem* set_item;
	for (HTQLItem* field=row; field; field=field->NextField){
		(*res) = new HTQLItem((*to_addr)?(*to_addr):field->ParentItem);
		set_item=0;
		if (field->ItemType == HTQLItem::itemSCHEMA_REF){ 
			set_item = (field->getSetItem());
			if (!set_item) set_item = field;
		}else{
			set_item = field;
		}
		(*res)->copyTag(set_item);
		(*res)->FieldName.Set(set_item->FieldName.P, set_item->FieldName.L, true);
		if (set_item->ItemType == HTQLItem::itemSCHEMA_REF) 
			(*res)->ItemType = HTQLItem::itemSET;
		else 
			(*res)->ItemType = set_item->ItemType;

		if (set_item->ItemType == HTQLItem::itemSCHEMA){
			(*res)->Data.Set(set_item->Data.P, set_item->Data.L, false);
		}else{
			(*res)->Data.Set(set_item->Data.P, set_item->Data.L, true);
		}
		res=&(*res)->NextField;
	}
	return 0;
}

long HTQLParser::copyAllRowsSetItem(HTQLItem* row, HTQLItem** to_addr){
	HTQLItem* data;
	HTQLItem** res=to_addr;

	while (*res) res = &(*res)->NextItem;

	long count=0;
	for (data=row; data; data=data->NextItem){
		copyRowSetItem(data, res);
		res = &(*res)->NextItem;
		count++;
	}
	return count;
}

int HTQLParser::printResultData(FILE* fw){
	fprintf(fw, "<html><body>");
	HTQLItem item(0);
	item.isRefNextItem = true;
	item.ItemType= HTQLItem::itemSCHEMA_REF;
	item.NextItem = Data;
	int res= HTQLItem::printHtqlItem(&item, fw);
	fprintf(fw, "</body></html>");
	return res;
}
int HTQLParser::printResultData(const char* filename){
	FILE* fw = fopen(filename, FILE_WRITE);
	if (!fw ) return -1;
	int res=printResultData(fw);
	fclose(fw);
	return res;
}

long HTQLParser::formatHtmlResult(const char* filename){
	FILE* fw = fopen(filename, FILE_WRITE);
	if (!fw ) return -1;
	int res=formatHtmlResult(fw);
	fclose(fw);
	return res;
}

long HTQLParser::formatHtmlResult(FILE* fw){
	fprintf(fw, "<html><body>");
	fprintf(fw, "<base href='%s'>", getSourceUrl() );
	int i=0, fieldsnum=getFieldsCount();
	long count=0;
	fprintf(fw, "<table border=1>");
	fprintf(fw, "<tr bgcolor=lightgreen>");
	char* p=0;
	for (i=1; i<=fieldsnum; i++){
		p=getFieldName(i);
		if (p&&*p){
			fprintf(fw, "<td>%s</td>", p);
		}else{
			fprintf(fw, "<td>%%%d</td>", i);
		}
	}
	fprintf(fw, "</tr>\n");
	
	moveFirst();
	while (!isEOF()){
		fprintf(fw, "<tr bfcolor=white>");
		for (i=1; i<=fieldsnum; i++){
			p= getField(i);
			fprintf(fw, "<td valign=top align=left>%s</td>", (p&&*p)?p:"<br>");
		}
		fprintf(fw, "</tr>\n");
		count++;
		moveNext();
	}

	fprintf(fw, "</table>\n");
	fprintf(fw, "<hr><b>Attributes</b>=%d, <b>tuples</b>=%ld, <b>total</b>=%ld<br>\n", fieldsnum, count, fieldsnum*count);
	fprintf(fw, "</body></html>");
	return count;
}

int HTQLParser::saveSourceData(const char* filename){
	FILE* fw = fopen(filename, FILE_WRITE);
	if (!fw ) return -1;
	int res=saveSourceData(fw);
	fclose(fw);
	return res;
}
int HTQLParser::saveSourceData(FILE* fw){
	fwrite(SourceData.P, 1, SourceData.L, fw);
	return 0;
}

int HTQLParser::functionSaveSourceFile(char* data, void* call_from, ReferData* tx){
	HTQLParser* call = (HTQLParser* )call_from;
	if (call->FunctionParameters && call->FunctionParameters->Value.P ){
		call->saveSourceData(call->FunctionParameters->Value.P);
	}
	return 0;
}

int HTQLParser::parseSchemaConstruct(){
	if (Syntax.Type != QLSyntax::synQL_WORD) return 0;
	Results = new HTQLItem(Data?Data->ParentItem:0);
	Results->ItemType =  HTQLItem::itemSCHEMA_DEF;
	HTQLItem** res=&Results;
	res= &(*res)->NextItem;

	//copy Data to Results
	HTQLItem* item; 
	HTQLItem* item1=0;
	item=Data;
	for (; item; item=item->NextItem){
		if (item->ItemType == HTQLItem::itemSCHEMA_DEF)
			continue;
		(*res) = new HTQLItem(item->ParentItem);
		item1 = (item->getSetItem());
		if (item1){
			(*res)->copyTag(item1);
			(*res)->ItemType = HTQLItem::itemSCHEMA;
			(*res)->Data.Set(item1->Data.P, item1->Data.L, false);

			//use item field name to store next fields;
			if (item->ItemType == HTQLItem::itemSCHEMA){
				(*res)->FieldName.Set((char*) item->NextField,0,false);
			}else {
				(*res)->FieldName.Set((char*) item,0,false);
			}
		}
		res= &(*res)->NextItem;
	}

	res=&Results;
	res= &(*res)->NextField;

	switchResults();
	//results are in Data now

	HTQLTag LastTag0; 	LastTag0.copyTag(&LastTag, true);
	TagOptions Options0 = Options;

	int i;
	while (Syntax.Type == QLSyntax::synQL_WORD){
		Options = Options0;
		LastTag.copyTag(&LastTag0, true);

		//construct schema
		(*res) = new HTQLItem(Results);
		(*res)->FieldName.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
		Syntax.match();
		if (Syntax.Type != QLSyntax::synQL_EQ) {
			delete (*res);
			(*res)=0;
			continue;
		}else{
			res= &(*res)->NextField;
			Syntax.match();
		}

		//calculate data
		HTQLItem* DataBack = Data;
		Data=0;

		//calculate each record field;
		HTQLItem* data1=0;
		HTQLSyntax syntax1;
		HTQLTag LastTag1;
		int ParaIndex=1;
		if (Syntax.Type == HTQLSyntax::synQL_PARA && Syntax.NextType == HTQLSyntax::synQL_NUMBER){
			Syntax.match();
			i=0;
			sscanf(Syntax.Sentence + Syntax.Start, "%d", &i);
			if (i>0) ParaIndex=i;
			Syntax.match();
		}
		syntax1.setSentence(Syntax.Data.P, &Syntax.Data.L, false);
		syntax1.copyFrom(&Syntax);
		LastTag1.copyTag(&LastTag, true);
		TagOptions Options1;
		Options1 = Options;
		for (item = DataBack->NextItem; item; item=item->NextItem){
			Data=new HTQLItem(item->ParentItem);
			if (ParaIndex<=1){
				Data->copyTag(item,false);
				Data->Data.Set(item->Data.P, item->Data.L, false);
	//			data1 = Data;
			}else{
				item1 = (HTQLItem*) item->FieldName.P;
				for (i=1; i<ParaIndex; i++){
					if (item1) item1=item1->NextField;
				}
				if (item1) item1=item1->getSetItem();
				if (item1) {
					Data->copyTag(item1,false);
					Data->Data.Set(item1->Data.P, item1->Data.L, false);
				}
			}
			IsMidResult = false;

			Syntax.copyFrom(&syntax1);
			LastTag.copyTag(&LastTag1, true);
			Options = Options1;
			i=parseSentence();
			if (i<0) return i;
			
			for (item1=item; item1->NextField; item1=item1->NextField);

			if (Results) switchResults();
			if (Data){
				if (Data->ItemType == HTQLItem::itemSET ||
					Data->ItemType == HTQLItem::itemSCHEMA_REF
					){
					item1->NextField = Data;
					item1=item1->NextField;
				}else if (Data->ItemType == HTQLItem::itemSCHEMA_DEF ||
					Data->ItemType == HTQLItem::itemSCHEMA
					){
					item1->NextField = new HTQLItem(item);
					item1=item1->NextField;
					item1->ItemType =  HTQLItem::itemSCHEMA_REF;
					item1->NextItem = Data;
				}
			}else{
				item1->NextField = new HTQLItem(item);
				item1 = item1->NextField;
			}
			Data=0;
//			delete data1;
//			data1=0;
		}

		//push Data to MidResult if there are.
		switchResults();

		//after the construction, results are stored in Data
		Data=DataBack;

		while (Syntax.Type == QLSyntax::synQL_SEMICOLON) Syntax.match();
	}

	return 0;
}

int HTQLParser::parseSchemaCondition(){
	if (Syntax.Type == QLSyntax::synQL_MID) return 0;

	tExprCalc expr;
	int i=expr.setExpression(Syntax.Sentence+Syntax.Start, 0);
	if (i<0) return i;
	i=expr.parse(synEXP_RIGHTBRACE);
	if (i<0) return i;

	copyRow(Data, &Results);

	HTQLItem* data;
	HTQLItem** res=&Results->NextItem;

	if (Data){
		for (data=Data->NextItem; data; data=data->NextItem){
			//set expression variables;

			setExprParam(Results, data, &expr);
			/*for (field=Results->NextField, item=data->NextField
				; field && item
				; field=field->NextField, item=item->NextField
				){
				item1 = getSetItem(item);
				if (item1){
					field->Data.Set(item1->Data.P, item1->Data.L, true);
				}else{
					field->Data.Set("",0, true);
				}
				expr.setField(field->FieldName.P, field->Data.P, field->Data.L);
			}*/

			expr.calculate();
			if (expr.getBoolean()){
				copyRow(data, res);
				res = &(*res)->NextItem;
			}
		}
	}

	Syntax.Next = Syntax.Start+expr.ExprSentence->Start;
	Syntax.NextLen = 0;
	Syntax.match();
	Syntax.match();

	// set results to Data;
	switchResults();
	return 0;
}

int HTQLParser::setExprParam(HTQLItem* field, HTQLItem* item, tExprCalc* expr, HTQLItem** firstfield){
	int i;
	char buf[10];
	for (i=0; i<expr->FieldsNum; i++){
		expr->FieldsList[i]->deleteValue();
	}
	for (tStack* t=GlobalVariables.Next; t; t=t->Next){
		expr->setField(t->Key, t->Value);
	}
	i=0;
	HTQLItem* para_field=field;
	int para_field_data=false;
	if (field && field->ItemType == HTQLItem::itemSCHEMA_DEF){
		para_field_data=true;

		if (!para_field->NextField && para_field!=item) para_field->NextField=new HTQLItem(para_field->ParentItem);
		para_field = para_field->NextField;
	}
	if (item && item->ItemType == HTQLItem::itemSCHEMA){
		item = item->NextField;
	}
	HTQLItem* item1;
	HTQLItem* first_field=0;
	ReferData field_value;
	while (item){
		field_value.reset();
		item1 = (item->getSetItem());
		if (item1) field_value.Set(item1->Data.P, item1->Data.L, false);

		if (para_field_data && item1!=para_field){
			if (field_value.P){
				para_field->Data.Set(field_value.P, field_value.L, true);
			}else{
				para_field->Data.reset();
				//field->Data.Set("",0, true);
			}
		}
		if (para_field->FieldName.P){
			if (expr->setField(para_field->FieldName.P, field_value.P, field_value.L)==0 && !first_field){
				first_field=item1?item1:item;
			}
		}
		i++;
		sprintf(buf, "%%%d", i);
		if (expr->setField(buf, field_value.P, field_value.L)==0 && !first_field){
				first_field=item1?item1:item;
		}
		if (!para_field->NextField && para_field!=item) para_field->NextField=new HTQLItem(para_field->ParentItem);
		para_field=para_field->NextField; 
		item=item->NextField; 
	}
	if (firstfield) *firstfield=first_field;
	return i; //field num;
}

int HTQLParser::parseSchemaTransform(){
	Results = new HTQLItem(Data?Data->ParentItem:0);
	Results->ItemType =  HTQLItem::itemSCHEMA_DEF;
	HTQLItem** res=&Results;
	res= &(*res)->NextItem;
	
	//copy Data to Results
	HTQLItem* item; 
	HTQLItem* item1=0;
	for (item=Data; item; item=item->NextItem){
		if (item->ItemType == HTQLItem::itemSCHEMA_DEF)
			continue;
		(*res) = new HTQLItem(item->ParentItem);
		item1 = (item->getSetItem());
		if (item1){
			(*res)->copyTag(item1);
			(*res)->ItemType = HTQLItem::itemSCHEMA;
			(*res)->Data.Set((char*)item, 0, false);
			//copy by reference.
		}
		res= &(*res)->NextItem;
	}

	res=&Results;
	res= &(*res)->NextField;

	//copy fields;
	HTQLItem* tmpfields=0;
	copyRow(Data, &tmpfields);

	ReferData rename;
	tExprCalc expr;
	HTQLItem *data;
	HTQLItem *field;
	//char buf[10];
	HTQLItem *offsetfield;
	char*p=0;
	while (Syntax.Type != QLSyntax::synQL_END && Syntax.Type != QLSyntax::synQL_RBRACKET){
		if (Syntax.Type == QLSyntax::synQL_WORD && Syntax.NextType == QLSyntax::synQL_EQ){
			rename.Set(Syntax.Sentence+Syntax.Start, Syntax.StartLen, true);
			Syntax.match(); //QLSyntax::synQL_WORD
			Syntax.match(); //QLSyntax::synQL_EQ
		}else{
			rename.Set("", 0, true);
		}
		int i=expr.setExpression(Syntax.Sentence+Syntax.Start, 0);
		if (i<0) return i;
		i=expr.parse(synEXP_RIGHTBRACE);
		if (i<0) return i;
		if (rename.L==0 && expr.FieldsNum > 0){
			if (expr.FieldsList[0]->Name && expr.FieldsList[0]->Name[0]!=preEXP_PARA){
				rename.Set(expr.FieldsList[0]->Name, strlen(expr.FieldsList[0]->Name),true);
			}else if (expr.FieldsList[0]->Name[0]!=preEXP_PARA){
				int index=atoi(expr.FieldsList[0]->Name+1);
				if (Data->ItemType==HTQLItem::itemSCHEMA_DEF){
					item=Data;
					for (i=0; i<index; i++){
						if (item) item=item->NextField;
					}
					if (item) rename.Set(item->FieldName.P, item->FieldName.L, true);
				}
			}
		}
		
		// create new schema field
		(*res)=new HTQLItem(Results);
		if (rename.L) (*res)->FieldName.Set(rename.P, rename.L, true);
		res = &(*res)->NextField;

		// create new data for new field
		for (data=Results->NextItem; data; data=data->NextItem){
			//set expression variables;
			item=(HTQLItem*)data->Data.P;
			field=tmpfields;

			setExprParam(field, item, &expr, &offsetfield);

			expr.calculate();
			p=expr.getString();
			for (item1=data; item1->NextField; item1=item1->NextField);
			item1->NextField=new HTQLItem(data);
			item1=item1->NextField;
			if (p) {
				item1->Data.Set(p, strlen(p), true);
				if (offsetfield) {
					item1->SourceOffset = offsetfield->SourceOffset;
					//item1->S.Set(offsetfield->S.P, offsetfield->S.L, false);
					//item1->E.Set(offsetfield->E.P, offsetfield->E.L, false);
				}else{
					//item1->S.Set(item1->Data.P, 0, false);
					//item1->E.Set(item1->Data.P+item1->Data.L, 0, false);
					if (item) item1->SourceOffset = item->SourceOffset;
					else item1->SourceOffset = data->SourceOffset;
				}
				//how to set S and E?? for UPDATE functiom ... 2003.07.10
					item1->S.Set(item1->Data.P, 0, false);
					item1->E.Set(item1->Data.P+item1->Data.L, 0, false);
			}
		}

		Syntax.Next = Syntax.Start+expr.ExprSentence->Start;
		Syntax.NextLen = 0;
		Syntax.match();
		Syntax.match();

		if (Syntax.Type == QLSyntax::synQL_SEMICOLON) {
			while (Syntax.Type == QLSyntax::synQL_SEMICOLON)
				Syntax.match();
		}else break;
	}

	// set results to Data;
	delete tmpfields;
	switchResults();
	return 0;
}

HTQLItem* HTQLParser::moveFirst(){
	CurrentRecord=Data;
	while (CurrentRecord && 
		(CurrentRecord->ItemType == HTQLItem::itemSCHEMA_DEF ||
		CurrentRecord->ItemType == HTQLItem::itemSCHEMA_REF ) 
		){
		CurrentRecord = CurrentRecord->NextItem;
	}
	return CurrentRecord;
}

HTQLItem* HTQLParser::moveNext(){
	if (CurrentRecord) CurrentRecord=CurrentRecord->NextItem;
	while (CurrentRecord && 
		(CurrentRecord->ItemType == HTQLItem::itemSCHEMA_DEF ||
		CurrentRecord->ItemType == HTQLItem::itemSCHEMA_REF ) 
		){
		CurrentRecord = CurrentRecord->NextItem;
	}
	return CurrentRecord;
}

HTQLItem* HTQLParser::movePrev(){
	HTQLItem* cur_rec = CurrentRecord;
	HTQLItem* this_item = moveFirst();
	HTQLItem* next_item;
	if (cur_rec == this_item){
		CurrentRecord = 0;
		return CurrentRecord;
	}
	while (this_item){
		next_item = moveNext();
		if (cur_rec == next_item ){
			CurrentRecord = this_item;
			return CurrentRecord;
		}
		this_item = next_item;
	}
	return CurrentRecord;
}

HTQLItem* HTQLParser::moveLast(){
	CurrentRecord = 0;
	return movePrev();
}
int HTQLParser::isBOF(){
	return !CurrentRecord; 
}
HTQLItem* HTQLParser::getFieldItem(int index){
	if (!CurrentRecord) return 0;
	HTQLItem* p=CurrentRecord;
	if (CurrentRecord->ItemType == HTQLItem::itemSCHEMA){
		if (index==0){
			p->Data.Seperate();
			return p;
		}
		for (int i=0; i<index; i++) {
			if (p)
				p = p->NextField;
		}
	}else if (CurrentRecord->ItemType == HTQLItem::itemSET){
		if (index > 1) 
			p=0;
	}
	return p?p->getSetItem():0;
}

char* HTQLParser::getField(int index){
	HTQLItem* p=getFieldItem(index);
	if (!p) return 0;
	p->Data.Seperate();
	return p->Data.P;
}

char* HTQLParser::getField(const char* name){
	if (!CurrentRecord) return 0;
	int index=0;
	HTQLItem* p=Data;
	if (Data->ItemType == HTQLItem::itemSCHEMA_DEF){
		if (name){
			for (p=Data->NextField; p; p=p->NextField){
				index++;
				if (!p->FieldName.Cmp(name, strlen(name), Options.Sen) )
					break;
			}
		}
		if (!p) return 0;
	}else{
		if (name && strcmp(name, "")) return 0;
	}
	return getField(index);
}

int HTQLParser::getFieldsCount(){
	if (!Data) return 0;
	HTQLItem* p;
	int index=0;
	if (Data->ItemType == HTQLItem::itemSCHEMA_DEF || Data->ItemType == HTQLItem::itemSCHEMA){
		for (p=Data->NextField; p && (!p->FieldName.isNULL() || !p->Data.isNULL()) ; p=p->NextField){
			index++;
		}
		return index;
	}else{
		return 1;
	}
}

long HTQLParser::getTuplesCount(){
	long count=0;
	moveFirst();
	while (!isEOF()){
		count++;
		moveNext();
	}
	moveFirst();
	return count;
}

char* HTQLParser::getFieldName(int index){
	if (!Data) return 0;
	HTQLItem* p=Data;
	if (Data->ItemType == HTQLItem::itemSCHEMA || Data->ItemType == HTQLItem::itemSCHEMA_DEF){
		if (index==0){
			p->FieldName.Seperate();
			return p->FieldName.P;
		}
		for (int i=0; i<index; i++) {
			if (p)
				p = p->NextField;
		}
	}else if (CurrentRecord&& CurrentRecord->ItemType == HTQLItem::itemSET){
		if (index > 1) 
			p=0;
	}
	if (p) p=(p->getSetItem());
	if (!p) return 0;
	p->FieldName.Seperate();
	return p->FieldName.P;	
}

int HTQLParser::setGlobalVariable(char* name, char* value){
	tStack* t=GlobalVariables.search(name,0);
	if (t){
		t->newValue(value);
	}else{
		GlobalVariables.set(0, name, value);
	}
	return 0;
}
void HTQLParser::resetGlobalVariables(){
	GlobalVariables.reset();
}
HTQLItem* HTQLParser::getDataItem(long row, long col){ //index from 1;
	long i,j;
	HTQLItem* item = Data;
	for (i=1; i<row && item; i++){
		item = item->NextItem;
	}
	if (!item) return 0;
	if (col==0) return item;
	if (item->ItemType == HTQLItem::itemSCHEMA) item = item->NextField;
	for (j=1; j<col && item; j++){
		item = item->NextItem;
	}
	return item;
}

int HTQLParser::dropDataItem(long row, long col){ //row, col: index from 1;
	HTQLItem** item = &Data;
	for (long i=1; i<row && (*item); i++){
		item = &(*item)->NextItem;
	}
	if (!*item) return 1;
	if (col==0) return HTQLItem::dropField(item, 1);
	if ((*item)->ItemType == HTQLItem::itemSCHEMA) item = &(*item)->NextField;
	return HTQLItem::dropField(item, col);
}

int HTQLParser::insertDataItem(long row, long col){ //row, col: index from 1;
	HTQLItem** item = &Data;
	for (long i=1; i<row; i++){
		if (!(*item)) *item = new HTQLItem(Data?Data->ParentItem:0);
		item = &(*item)->NextItem;
	}
	return HTQLItem::insertField(item, col, Data?Data->ParentItem:0);
}

int HTQLParser::setDataItemData(long row, long col, char* p, long len, int copy){ //row, col: index from 1;
	HTQLItem** item = &Data;
	for (long i=1; i<row; i++){
		if (!(*item)) *item = new HTQLItem(Data?Data->ParentItem:0);
		item = &(*item)->NextItem;
	}
	return HTQLItem::setFieldData(item, col, p, len, copy, (*item)?(*item):Data->ParentItem);
}

int HTQLParser::setDataItemName(long row, long col, char* name){ //row, col: index from 1;
	HTQLItem** item = &Data;
	for (long i=1; i<row; i++){
		if (!(*item)) *item = new HTQLItem(Data?Data->ParentItem:0);
		item = &(*item)->NextItem;
	}
	return HTQLItem::setFieldName(item, col, name, (*item)?(*item):Data->ParentItem);
}

int HTQLParser::dropCurrentRecordItem(long col){ //row, col: index from 1;
	if (!CurrentRecord) return 1;
	if (CurrentRecord->ItemType == HTQLItem::itemSCHEMA && col) col++;

	HTQLItem** item = &Data;
	while (*item && *item != CurrentRecord) 
		item = &(*item)->NextItem;
	if (!*item) return 1;

	int err= HTQLItem::dropField(item, col);
	CurrentRecord = *item;
	return err;
}

int HTQLParser::dropCurrentRecord(){ //row, col: index from 1;
	if (!CurrentRecord) return 1;

	HTQLItem** item = &Data;
	while (*item && *item != CurrentRecord) 
		item = &(*item)->NextItem;
	if (!*item) return 1;

	*item = CurrentRecord->NextItem;
	CurrentRecord->NextItem = 0;
	delete CurrentRecord;
	CurrentRecord = *item;
	return 0;
}

int HTQLParser::insertCurrentRecordItem(long col){ //row, col: index from 1;
	if (!CurrentRecord) return 1;
	if (CurrentRecord->ItemType == HTQLItem::itemSCHEMA && col) col++;

	HTQLItem** item = &Data;
	while (*item && *item != CurrentRecord) 
		item = &(*item)->NextItem;
	if (!*item) return 1;

	int err= HTQLItem::insertField(item, col, *item);
	CurrentRecord = *item;
	return err;
}

int HTQLParser::mergeCurrentRecordItem(long col){ //row, col: index from 1;
	if (!CurrentRecord) return 1;
	if (CurrentRecord->ItemType == HTQLItem::itemSCHEMA && col) col++;

	HTQLItem** item = &Data;
	while (*item && *item != CurrentRecord) 
		item = &(*item)->NextItem;
	if (!*item) return 1;

	int err= HTQLItem::mergeField(item, col);
	CurrentRecord = *item;
	return err;
}

int HTQLParser::setCurrentRecordItemData(long col, char* p, long len, int copy){ //row, col: index from 1;
	if (!CurrentRecord) return 1;
	if (CurrentRecord->ItemType == HTQLItem::itemSCHEMA && col) col++;

	HTQLItem** item = &Data;
	while (*item && *item != CurrentRecord) 
		item = &(*item)->NextItem;
	if (!*item) return 1;

	int err= HTQLItem::setFieldData(item, col, p, len, copy, *item);
	CurrentRecord = *item;
	return err;
}

int HTQLParser::setCurrentRecordItemName(long col, char* name){ //row, col: index from 1;
	if (!CurrentRecord) return 1;
	if (CurrentRecord->ItemType == HTQLItem::itemSCHEMA && col) col++;

	HTQLItem** item = &Data;
	while (*item && *item != CurrentRecord) 
		item = &(*item)->NextItem;
	if (!*item) return 1;

	int err= HTQLItem::setFieldName(item, col, name, *item);
	CurrentRecord = *item;
	return err;
}

int HTQLParser::isEOF(){
	return !CurrentRecord; 
}


