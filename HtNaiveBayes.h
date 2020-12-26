#ifndef CLY_HT_NAIVE_BAYES_H_20140823
#define CLY_HT_NAIVE_BAYES_H_20140823

#include "referdata.h"
#include "referlink.h"

class ReferSet; 

class HtNaiveBayes {
public: 
	ReferData ValueSep;
	ReferData NoneStr;
	double Smooth; 
	ReferLinkHeap FeatureNames;
	ReferData OutcomeName; 
	long NSamples; 
	long NClasses;
	ReferLinkHeap LabelFreqDist; //Name: label, Data: count
	ReferLinkHeap FeatureValues; //Name: fname, Data: (ReferLinkHeap*) (Name: fval) 
	ReferLinkHeap FeatureFreqdisDict; //Name: label, Value: fname, Data: (ReferLinkHeap*) (Name: fval, Data: count)

	int fitData(ReferSet* data, ReferLinkHeap* feature_names, ReferData* outcome_name );
	double predictProba(ReferData* tuple, ReferLinkHeap* probvals, int to_print=false);
	int dumpXML(ReferData* data);
	int loadXML(ReferData* data); 
	int saveFile(const char* filename);
	int loadFile(const char* filename); 

	int splitValues(const char* val, ReferLinkHeap* vals);
	int addFreqDist(ReferData* fname, ReferData* fval, ReferData* label, long count);
	int addFeatureValue(ReferData* fname, ReferData* fval);

	double addLogs(double logx, double logy);
	double sumLogs(ReferLinkHeap* logvals);
public: 
	HtNaiveBayes();
	~HtNaiveBayes();
	void reset(); 
}; 

#endif

