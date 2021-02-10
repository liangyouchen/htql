#ifndef CLY_HTWRAPPER_H_20030606
#define CLY_HTWRAPPER_H_20030606

#include "referdata.h"
#include "htmlql.h"

class ReferLinkHeap;

#define dhyconPLAIN		'D'
#define dhyconS_TAG		'T'
#define dhyconE_TAG		't'
#define dhyconS_TAG_A		'A'
#define dhyconE_TAG_A		'a'
#define dhyconS_TAG_B		'B'
#define dhyconE_TAG_B		'b'
#define dhyconS_TAG_FONT		'F'
#define dhyconE_TAG_FONT		'f'
#define dhyconS_TAG_IMAGE		'M'
#define dhyconE_TAG_IMAGE		'm'

#define dtxconWORD		'W'
#define dtxconNUMBER	'N'
#define dtxconKEY		'K'
#define dtxconDATE		'D'
#define dtxconCURRENCY	'$'
#define dtxconTIME		'T'
#define dtxconSYMBOLS	":;|@%%#."

class HtWrapper {
public:
	ReferData WrapperFileName;
	HtmlQL Source;
	ReferData Wrapper;
	ReferData WrapperName;

	double ValidationThreshold;

	//create wrapper
	int emptyWrapper();
	int newWrapper(const char* wrapper_name);
	int readWrapper(const char* filename);
	int saveWrapper(const char* filename=0);

	//set source data
	int setSourceData(char* source, long len, int copy=false);
	int setSourceUrl(char* url);
	int setSource(HTQL* htql, int copy=false);

	//wrap
	HTQL* wrapSourceData(const char* wrapper_name=0, int auto_adjust=false);
			// set WrapperBackup if auto_adjust
			// set IsWrapperAdjusted, IsAdjustedByCandidateHtql, IsAdjustedByCandidatePattern
	int IsWrapperAdjusted;
	int IsAdjustedByCandidateHtql;
	int IsAdjustedByCandidatePattern;
	ReferData WrapperBackup;

	//validate
	double validateWrapper(HTQL* qresult, const char* wrapper_name=0, double con_threshold=0.3, int* invalid_fields=0);
			// qresult: an HTQL query data; con_threshold: consolidation threshold; 
			// returns a validation score;
			// set ValidationAllSimil, ValidationFieldsNum and ValidationFieldsSimil[];
	double* ValidationFieldsSimil;
	int ValidationFieldsNum;
	double ValidationAllSimil;

	//automatic wrapper creation
	//check_fields: ture -- compute field consensus info; 
	//			= setWrapperConFields + removeTrivialConFields;
	int setWrapperByBestView(const char* wrapper_name=0, int check_fields=false);
	int setWrapperByCandidateView(const char* candidate_htql, const char* wrapper_name=0, int check_fields=false);
	int setWrapperByTargetItem(const char* candidate_htql, const char* wrapper_name=0, int check_fields=false);

	int setWrapperConFields(int max_comp_tuple=0);
	int removeTrivialConFields();

	int updateWrapperHtql(const char* view_type, const char* htql, const char* candidate);
			//update htql, candidate, and candidate path and patterns
			//view_type: "View" -- set CandidateHtql and CandidatePattern; 
			//			 others -- set CandidateHtql;
	int replaceWrapperHtql(const char* htql=0, const char* candidate=0);
			//update only the htql and candidate

	int adjustWithCandidateHtqls();
			//set IsWrapperAdjusted, BestWrapperAdjustedScore and BestWrapperAdjusted;
	int adjustWithCandidatePatterns();
			//set IsWrapperAdjusted, BestWrapperAdjustedScore and BestWrapperAdjusted;
	ReferData BestWrapperAdjusted;
	double BestWrapperAdjustedScore;

public:
	static int consolidateConWrapper(ReferData* original_wrapper, ReferData* current_wrapper, char* wrapper_name, ReferData* result_wrapper, double** evals=0, int* fieldsnum=0);
	static int computeFieldHyperConStr(HTQL* htql, int field_index1, ReferData* consen_str, double* cost, long max_comp_len=10, int max_comp_tuple=0);
	static int computeFieldTextConStr(HTQL* htql, int field_index1, ReferData* consen_str, double* cost, long max_comp_len=10, int max_comp_tuple=0);
	static int computeFieldConStr(HTQL* htql, int field_index1, ReferData* consen_str, double* cost, int (*constr_fun)(char*, char*, long*, long), long max_comp_len, int max_comp_tuple );
	static int buildHyperConStr(char* p, char* syntax_buf, long*pos_buf, long max_len);
	static int buildTextConStr(char* p, char* syntax_buf, long*pos_buf, long max_len);
	static int buildPlainConStr(char* p, char* syntax_buf, long*pos_buf, long max_len);
	static int computeFieldLengthStatistic(HTQL* htql, int field_index1, long* max_len, long* min_len, long* average_len, long* tuple_count, int max_comp_tuple=0);

public:
	HtWrapper();
	~HtWrapper();
	void reset();

protected:
};

#endif
