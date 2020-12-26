#ifndef HTWRAPPER_MODELS_H_CLY_2006_12_24
#define HTWRAPPER_MODELS_H_CLY_2006_12_24

#include "referset.h"

class HTQL;


class HtWrapperModels{
	//see: HtWrapperModels.doc
public:
	enum{
		ID_MODEL_ID=0, ID_MODEL_TYPE, ID_PATTERN_TYPE, ID_PATTERN, ID_OPTIONS, ID_MODEL_FIELDS_NUM,
		/*ID_MODEL_ID=0,*/ ID_PAGE_SOURCE=1, ID_PAGE_URL, ID_PAGE_SAVED_FILE, ID_PAGE_FIELDS_NUM,
	};
	/*	ID_MODEL_TYPE: "Global", "Local", "Validation"
		ID_PATTERN_TYPE: "SelectedHTQL", "HtqlPrefix", "HtqlSchema", "Text", "TagPattern", "TextPattern"
	*/
	enum{
		PAT_HTQL=0x01, 
		PAT_HTQL_SCHEMA=0x02, 
		PAT_TEXT=0x04, 
		PAT_TAG_PATTERN=0x08, 
		PAT_TEXT_PATTERN=0x10,
		PAT_HTQL_NEXT=0x20
	};
	ReferSet Models; 
	ReferSet Pages;
	int MaxLocalModels;
	int setupInitialModels();

	ReferData* addNewPage(ReferData* url, ReferData*page, /*in,out*/ReferData* model_id);
	int addNewModels(ReferData*model_id, ReferData* htql, long pattern_type);
	double validateWithModel(ReferData*page, ReferData* htql, ReferData* model_id=0, long pattern_type=0xFF);
	long findBetterMatchPos(ReferData*page, ReferData* htql, ReferData* model_id=0, long pattern_type=0xFF, double* score=0);
	int selectAlternativeHtql(ReferData*page, long target_pos, ReferData* model_id=0, ReferData* selected_htql=0);
	int relocateWithModel(ReferData* page, ReferData* htql, ReferData* results);
	int createGlobalModelFromLocal(ReferData* model_id); //the most recent model_id for test
	int dropWorseModel(ReferData*page, ReferData* htql);

protected:
	int createLocalModelHtql(ReferData*model_id, HTQL* ql, ReferData* htql);
	int createLocalModelHtqlNext(ReferData*model_id, HTQL* ql, ReferData* htql);
	int createLocalModelHtqlSchema(ReferData*model_id, ReferData* htql);
	int createLocalModelTagPattern(ReferData*model_id, HTQL* ql);
	int createLocalModelTextPattern(ReferData*model_id, HTQL* ql);
	int createLocalModelText(ReferData*model_id, HTQL* ql);
	double validateModelHtql(ReferData*page, ReferData* htql, ReferData* model_id=0);
	double validateModelHtqlSchema(ReferData*page, ReferData* htql, ReferData* model_id=0);
	double validateModelTagPattern(ReferData*page, ReferData* htql, ReferData* model_id=0);
	double validateModelTextPattern(ReferData*page, ReferData* htql, ReferData* model_id=0);
	double validateModelText(ReferData*page, ReferData* htql, ReferData* model_id=0);
	int getConsensusTagPattern(HTQL* ql, ReferData* pattern);
	int getConsensusTextPattern(HTQL* ql, ReferData* pattern);
	int getConsensusText(HTQL* ql, ReferData* pattern);
	double scoreConsensusPattern(ReferData* pattern, const char* pattern_select_sql);
	long locateModelPattern(ReferData* page, int (*constr_fun)(char*, char*, long*, long), const char* model_sql, ReferData* results);
public:
	HtWrapperModels();
	~HtWrapperModels();
	void reset();
};

#endif

