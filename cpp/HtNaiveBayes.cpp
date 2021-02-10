#include "HtNaiveBayes.h"

#include <math.h>
#include "stroper.h"
#include "referset.h"
#include "htmlql.h"

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define DEBUG_THIS_FILE
#endif


double _ADD_LOGS_MAX_DIFF = log(1e-30);

HtNaiveBayes::HtNaiveBayes(){
	ValueSep = "|";
	NoneStr = "0";
	Smooth = 0.5; 
	FeatureFreqdisDict.setSortOrder(SORT_ORDER_KEY_VAL_STR_INC);
	FeatureFreqdisDict.setDuplication(false);
	FeatureFreqdisDict.setCaseSensitivity(true); 
	NSamples=0;
	NClasses=0;
}

HtNaiveBayes::~HtNaiveBayes(){
	reset();
}

void HtNaiveBayes::reset(){
	ReferLink* link; 
	for (link=FeatureFreqdisDict.getReferLinkHead(); link; link=link->Next){
		if (link->Data){
			delete (ReferLinkHeap*) link->Data; 
			link->Data = 0; 
		}
	}
	FeatureFreqdisDict.empty(); 
	for (link=FeatureValues.getReferLinkHead(); link; link=link->Next){
		if (link->Data){
			delete (ReferLinkHeap*) link->Data; 
			link->Data = 0; 
		}
	}
	FeatureValues.empty(); 
	LabelFreqDist.empty();
	NSamples=0;
	NClasses=0;
	OutcomeName.reset();
	FeatureNames.empty();
}
int HtNaiveBayes::dumpXML(ReferData* data){
	ReferLink* link; 
	char buf[512]; 
	ReferData val; 
	*data = "<NaiveBayes>\n";
	sprintf(buf, "\t<Parameters ValueSep=\"%s\" NoneStr=\"%s\" Smooth=\"%.2f\" NSamples=\"%ld\" NClasses=\"%ld\"/>\n",  ValueSep.P, NoneStr.P, Smooth, NSamples, NClasses); 
	*data += buf;

	tStrOp::encodeHtml(OutcomeName.P, &val); 
	*data += "\t<OutcomeName>"; *data += val; *data += "</OutcomeName>\n";

	*data += "\t<FeatureNames>\n"; 
	for (link=(ReferLink*) FeatureNames.moveFirst(); link; link=(ReferLink*) FeatureNames.moveNext()){
		sprintf(buf, "\t\t<Feature Index=\"%ld\" Name=\"", link->Data); 
		*data+=buf; 
		tStrOp::encodeHtml(link->Name.P, &val); 
		*data+=val; *data+="\"/>\n";
	}
	*data += "\t</FeatureNames>\n";
	
	*data += "\t<LabelFreq>\n"; 
	for (link=(ReferLink*) LabelFreqDist.moveFirst(); link; link=(ReferLink*) LabelFreqDist.moveNext()){
		sprintf(buf, "\t\t<Label N=\"%ld\">", link->Data); 
		*data+=buf; 
		tStrOp::encodeHtml(link->Name.P, &val); 
		*data+=val; *data+="</Label>\n";
	}
	*data += "\t</LabelFreq>\n";

	*data += "\t<FeatureValues>\n"; 
	for (link=(ReferLink*) FeatureValues.moveFirst(); link; link=(ReferLink*) FeatureValues.moveNext()){
		tStrOp::encodeHtml(link->Name.P, &val); 
		*data+="\t\t<Feature Name=\""; *data+=val; *data+="\">\n";
		ReferLinkHeap* fvals = (ReferLinkHeap*) link->Data; 
		if (fvals){ 
			for (ReferLink* link1=(ReferLink*) fvals->moveFirst(); link1; link1=(ReferLink*) fvals->moveNext()){
				*data+="\t\t\t<Value>"; 
				tStrOp::encodeHtml(link1->Name.P, &val); 
				*data+=val; 
				*data+="</Value>\n";
			}
		}
		*data+="\t\t</Feature>\n";
	}
	*data += "\t</FeatureValues>\n";


	*data += "\t<FeatureFreq>\n"; 
	for (link=(ReferLink*) FeatureFreqdisDict.moveFirst(); link; link=(ReferLink*) FeatureFreqdisDict.moveNext()){
		tStrOp::encodeHtml(link->Value.P, &val); 
		*data+="\t\t<Feature Label=\""; *data += link->Name; *data+="\" Name=\""; *data+=val; *data+="\">\n";
		ReferLinkHeap* fvals = (ReferLinkHeap*) link->Data; 
		if (fvals){ 
			for (ReferLink* link1=(ReferLink*) fvals->moveFirst(); link1; link1=(ReferLink*) fvals->moveNext()){
				sprintf(buf, "\t\t\t<Value N=\"%ld\">", link1->Data);
				*data+=buf; 
				tStrOp::encodeHtml(link1->Name.P, &val); 
				*data+=val; 
				*data+="</Value>\n";
			}
		}
		*data+="\t\t</Feature>\n";
	}
	*data += "\t</FeatureFreq>\n";


	*data += "</NaiveBayes>";
	return 0;
}

int HtNaiveBayes::loadXML(ReferData* data){
	HtmlQL ql; 
	ql.setSourceData(data->P, data->L, false); 
	
	reset(); 
	ReferData val;
	ql.setQuery("<NaiveBayes>.<Parameters>: ValueSep, NoneStr, Smooth, NSamples, NClasses");
	if (!ql.isEOF()){
		ValueSep = ql.getValue("ValueSep");
		NoneStr = ql.getValue("NoneStr");
		val = ql.getValue("Smooth");
		Smooth = val.getDouble(); 
		val = ql.getValue("NSamples");
		NSamples = val.getLong();
		val = ql.getValue("NClasses");
		NClasses = val.getLong();
	}

	ql.setQuery("<NaiveBayes>.<OutcomeName>: tx");
	if (!ql.isEOF()){
		OutcomeName = ql.getValue(1); 
	}

	ql.setQuery("<NaiveBayes>.<FeatureNames>.<Feature> {name=:Name &tx; index=:Index} ");
	for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
		char* name=ql.getValue("name"); 
		val = ql.getValue("index"); 
		FeatureNames.add(name, 0, val.getLong());
	}

	ql.setQuery("<NaiveBayes>.<LabelFreq>.<Label> {name=:tx &tx; N=:N} ");
	for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
		char* name=ql.getValue("name"); 
		val = ql.getValue("N"); 
		LabelFreqDist.add(name, 0, val.getLong());
	}

	ql.setQuery("<NaiveBayes>.<FeatureValues>.<Feature> {name=:Name &tx; tx=:tx} ");
	for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
		char* name=ql.getValue("name"); 
		char* vals = ql.getValue("tx"); 
		ReferLink* link=(ReferLink*) FeatureValues.add(name, 0, 0);
		ReferLinkHeap* fvals = new ReferLinkHeap();
		link->Data = (long) fvals;
		HtmlQL ql1; 
		ql1.setSourceData(vals, strlen(vals), false);
		ql1.setQuery("<Value>:tx &tx"); 
		for (ql1.moveFirst(); !ql1.isEOF(); ql1.moveNext()){
			val=ql1.getValue(1);
			fvals->add(&val, 0, 0);
		}
	}

	ql.setQuery("<NaiveBayes>.<FeatureFreq>.<Feature> {label=:Label; name=:Name &tx; tx=:tx} ");
	for (ql.moveFirst(); !ql.isEOF(); ql.moveNext()){
		char* label=ql.getValue("label");
		char* name=ql.getValue("name"); 
		char* vals = ql.getValue("tx"); 
		ReferLink* link=(ReferLink*) FeatureFreqdisDict.add(label, name, 0);
		ReferLinkHeap* fvals = new ReferLinkHeap();
		link->Data = (long) fvals;
		HtmlQL ql1; 
		ql1.setSourceData(vals, strlen(vals), false);
		ql1.setQuery("<Value>{tx=:tx &tx; count=:N}"); 
		for (ql1.moveFirst(); !ql1.isEOF(); ql1.moveNext()){
			char* val1=ql1.getValue("tx");
			val = ql1.getValue("count"); 
			fvals->add(val1, 0, val.getLong() );
		}
	}
	return 0;
}

int HtNaiveBayes::saveFile(const char* filename){
	ReferData data;
	dumpXML(&data);
	return data.saveFile(filename);
}
int HtNaiveBayes::loadFile(const char* filename){
	ReferData data; 
	data.readFile(filename); 
	return loadXML(&data);
}

int HtNaiveBayes::splitValues(const char* val, ReferLinkHeap* vals){
	vals->setSortOrder(SORT_ORDER_NUM_INC);
	tStrOp::splitString(val, ValueSep.P, vals); 
	if (vals->Total==0) vals->add(&NoneStr, 0, 0);
	return 0;
}

int HtNaiveBayes::addFreqDist(ReferData* fname, ReferData* fval, ReferData* label, long count){
	ReferLink* link=FeatureFreqdisDict.find(label, fname, 0); 
	ReferLinkHeap* freq=0;
	if (!link){ 
		link = FeatureFreqdisDict.add(label, fname, 0); 
		freq = new ReferLinkHeap; 
		freq->setSortOrder(SORT_ORDER_KEY_STR_INC);
		freq->setCaseSensitivity(true);
		freq->setDuplication(false); 
		link->Data = (long) freq; 
	}
	freq = (ReferLinkHeap*) link->Data;
	if (freq){
		ReferLink* link1 = freq->findName(fval); 
		if (!link1) link1 = freq->add(fval, 0, count); 
		else link1->Data += count; 
		//assert link1->Data >0; 
	}
	return 0;
}
int HtNaiveBayes::addFeatureValue(ReferData* fname, ReferData* fval){
	ReferLink* link=FeatureValues.findName(fname); 
	ReferLinkHeap* vals=0;
	if (!link) {
		link=FeatureValues.add(fname, 0, 0); 
		vals=new ReferLinkHeap; 
		link->Data = (long) vals; 
	}
	vals= (ReferLinkHeap*) link->Data; 
	vals->add(fval, 0, 0); 
	return 0;
}

int HtNaiveBayes::fitData(ReferSet* data, ReferLinkHeap* feature_names, ReferData* outcome_name ){
	// feature_name->Value == 'N': numeric feature, with mult-values; otherwise: binary string value 
	reset();

	long feature_i=0; 
	ReferData* tuple=0; 
	ReferLink* link=0; 
	ReferLink* fname_link=0;
	for (link=feature_names->getReferLinkHead(); link; link=link->Next){
		feature_i=data->getFieldIndex(&link->Name);
		FeatureNames.add(&link->Name, &link->Value, feature_i);
	}

	OutcomeName.Set(outcome_name->P, outcome_name->L, true); 

	long outcome_i=data->getFieldIndex(outcome_name); 
	for (tuple=data->moveFirst(); tuple; tuple=data->moveNext()){
		link=LabelFreqDist.findName(&tuple[outcome_i]); 
		if (!link) link = LabelFreqDist.add(&tuple[outcome_i], 0, 0); 
		link->Data += 1; 
	}
	NSamples = data->TupleCount; 
	NClasses = LabelFreqDist.Total; 

	// Count up how many times each feature value occurred, given
	// the label and featurename.
	// cc=0;
	ReferLinkHeap fvals;
	for (tuple=data->moveFirst(); tuple; tuple=data->moveNext()){
		for (link=FeatureNames.getReferLinkHead(); link; link=link->Next){
			feature_i=link->Data;
			if (feature_i<0) continue; //field not found
			fvals.empty(); 
			splitValues(tuple[feature_i].P, &fvals); 
			if (!fvals.Total) fvals.add("", 0, 0); 
			for (ReferLink* link1 = fvals.getReferLinkHead(); link1; link1=link1->Next){
				addFreqDist(&link->Name, &link1->Name, &tuple[outcome_i], 1); //fname, fval, label, 1
				addFeatureValue(&link->Name, &link1->Name);
			}
		}
	}

	//
	// If a feature didn't have a value given for an instance, then
	// we assume that it gets the implicit value 'None.'  This loop
	// counts up the number of 'missing' feature values for each
	// (label,fname) pair, and increments the count of the fval
	// 'None' by that amount.
	ReferData fval; 
	fval="";
	ReferLink* label_link=0;
	for (label_link=(ReferLink*) LabelFreqDist.moveFirst(); label_link; label_link=(ReferLink*) LabelFreqDist.moveNext()){
		for (fname_link=FeatureNames.getReferLinkHead(); fname_link; fname_link=fname_link->Next){
			link=FeatureFreqdisDict.find(&label_link->Name, &fname_link->Name, 0); 
			if (!link) {
				addFreqDist(&fname_link->Name, &fval, &label_link->Name, label_link->Data);
				addFeatureValue(&fname_link->Name, &fval);
			}
		}
	}
	return 0; 
}

double HtNaiveBayes::predictProba(ReferData* tuple, ReferLinkHeap* probvals, int to_print){
	ReferLinkHeap logprob0; 
	ReferLink* label_link=0;
	ReferLink* link=0;
	for (label_link=(ReferLink*) LabelFreqDist.moveFirst(); label_link; label_link=(ReferLink*) LabelFreqDist.moveNext()){
		double lp=log(label_link->Data+Smooth)-log(NSamples+NClasses*Smooth);
		link=logprob0.add(&label_link->Name, 0, 0);
		link->Value.Malloc(sizeof(double));
		*((double*) link->Value.P) = lp; 
	}
	ReferLink* fname_link=0;
	ReferLinkHeap fvals; 
	for (fname_link=FeatureNames.getReferLinkHead(); fname_link; fname_link=fname_link->Next){
		if (fname_link->Data<0) continue;
		long feature_i = fname_link->Data; 
		fvals.empty(); 
		splitValues(tuple[feature_i].P, &fvals); 
		for (ReferLink* fval_link=fvals.getReferLinkHead(); fval_link; fval_link=fval_link->Next){
			ReferLink* cval_link=FeatureValues.findName(&fname_link->Name);
			if (!cval_link || !cval_link->Data || !((ReferLinkHeap*) cval_link->Data)->findName(&fval_link->Name)) 
				continue;
			for (label_link=(ReferLink*) LabelFreqDist.moveFirst(); label_link; label_link=(ReferLink*) LabelFreqDist.moveNext()){
				long fval_count=0; 
				link=FeatureFreqdisDict.find(&label_link->Name, &fname_link->Name, 0); 
				if (link && link->Data) {
					ReferLinkHeap* freq=(ReferLinkHeap*) link->Data; 
					ReferLink* link1=freq->findName(&fval_link->Name); 
					if (link1){
						fval_count = link1->Data; 
					}
				}
				long nvals=2;
				if (fname_link->Value.P && fname_link->Value.P[0]=='N'){
					nvals=((ReferLinkHeap*) cval_link->Data)->Total;
				}
				double lp = log(fval_count+Smooth)-log(label_link->Data+nvals*Smooth); 
				link=logprob0.findName(&label_link->Name);
				*((double*) link->Value.P) += lp; 
			}
		}
	}
	double sumlog = sumLogs(&logprob0);
	double last_prob=0.0;
	if (probvals){
		for (label_link=(ReferLink*) logprob0.moveFirst(); label_link; label_link=(ReferLink*) logprob0.moveNext()){
			link = probvals->findName(&label_link->Name);
			if (!link) link = probvals->add(&label_link->Name, 0, 0);
			link->Value.Malloc(sizeof(double)); 
			last_prob = exp( *((double*) label_link->Value.P) - sumlog ); 
			*((double*) link->Value.P) = last_prob;
		}
	}else{
		label_link=(ReferLink*) logprob0.moveLast();
		last_prob = exp( *((double*) label_link->Value.P) - sumlog );
	}
	return last_prob; 
}


double HtNaiveBayes::addLogs(double logx, double logy){
	if (logx < logy + _ADD_LOGS_MAX_DIFF) return logy;
	if (logy < logx + _ADD_LOGS_MAX_DIFF) return logx; 
	double base = (logx<logy)?logx:logy; 
	return base + log(exp(logx-base) + exp(logy-base));
}

double HtNaiveBayes::sumLogs(ReferLinkHeap* logvals){
	double lp=2*_ADD_LOGS_MAX_DIFF;
	for (ReferLink* link=logvals->getReferLinkHead(); link; link=link->Next){
		lp = addLogs(lp, *((double*) link->Value.P));
	}
	return lp; 
}








