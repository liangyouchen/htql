#ifndef CLY_QHTMLQL_H_20030610
#define CLY_QHTMLQL_H_20030610

#include "qhtql.h"
#include "htmlql.h"

class PHyperBase;

struct FindTagHtqlStruct;

enum {
	algFindTagHtqlChild2Parent, //using findTagHtqlChild2Parent algorithm
	algFindTagHtqlParent2Child		//using findTagHtqlParent2Child algorithm
};
extern int HtmlQLFindAlgorithm;
extern int HtmlQLFindLevel;

class HtmlQLParser: public HTQLParser{
public:
	int* FindHtqlPageMark; //pointer only, caller free memory
	int FindHtqlPageMarkLevel; 
	static int findHtql(ReferData* page, long offset, ReferLinkHeap* results, long heuristic=HtmlQL::heuBEST, long find_option=LEAVE_SINGLE_ITEM, int* mark=0, int mark_level=0);

public:
	static int functionRefineNullFields(char*p, void* call_from, ReferData* tx);//&refine_nullfields(threshold)
	static int functionFormatHtqlResults(char*p, void* call_from, ReferData* tx);//&FORMAT_HTML_RESULTS(to_encode, max_tuple)

	static int functionHtqlMaxViewPos(char* p, void* call_from, ReferData* tx); //&htql_bestviewpos
	//<htql_max_view_pos>: Htql, MaxTagPos, MaxTagNum, SecMaxTagPos, SecMaxTagNum, Similarity, TupleScore

	static int functionFindRepeatViews(char*p, void* call_from, ReferData* tx); //&find_repeat_views(show_num, search_num)
	//<find_repeat_views>.<find_view>:htql, candidate, view_items

	static int functionFindBestViews(char*p, void* call_from, ReferData* tx); //&find_best_views(show_num, search_num)
	//<find_best_views>.<find_view>:htql, candidate, view_items

	static int functionFormInputs(char* p, void* call_from, ReferData* tx); //&form_inputs
		//<form_inputs>:action, method 
		//<form_inputs>.<form_input>
		//	{name=:name; id=:id; value=:value; tag=:tag; type=:type; desc=:tx;}

	static int functionTagParent(char* p, void* call_from, ReferData* tx); //&tag_parent(tag_name);
		//the parent tag
	static int functionIEPosTag(char* p, void* call_from, ReferData* tx); //tag_iepos(index1)
		//the tag

	static int functionTagNext(char* p, void* call_from, ReferData* tx); //&tag_next
	static int functionTagPrevious(char* p, void* call_from, ReferData* tx); //&tag_previous

	static int functionTagAttrs(char*p, void* call_from, ReferData* tx); //&tag_attrs
		//<tag_attrs>:tag_name
		//<tag_attrs>.<tag_attr>{name=:name; value=:value;}

	static int functionFindHtql(char* p, void* call_from, ReferData* tx); //&find_htql(heurisic=heuBEST, leave_options=LEAVE_SINGLE_ITEM)
		//HTQL lines seperated by new line;

	static int functionFindFocusHtql(char* p, void* call_from, ReferData* tx); //&find_focus_htql

	static int functionTagLeaves(char*p, void* call_from, ReferData* tx); //&tag_leaves
		//<tag_leaves> 
		//<tag_leaves>.<tag_leave>{name=:name; tag_s_from=:tag_s_from; tag_s_len=:tag_s_len; tag_e_from=:tag_e_from; tag_e_len=:tag_e_len };

	static int functionFindLeavesHtql(char*p, void* call_from, ReferData* tx); //&find_leaves_htql(leave_options=LEAVE_SINGLE_ITEM)
		//<leaves_htql>
		//<leaves_htql>.<leave_htql> {name=:name; htql=:htql;} 

	static int functionTagLeavesHtql(char*p, void* call_from, ReferData* tx); //&tag_leaves_htql(leave_options=LEAVE_SINGLE_ITEM)
		//<leaves_htql>
		//<leaves_htql>.<leave_htql> {name=:name; htql=:htql;} 
	static int functionPlainLeavesHtql(char*p, void* call_from, ReferData* tx); //&plain_leaves_htql
		//<leaves_htql>
		//<leaves_htql>.<leave_htql> {name=:name; htql=:htql;} 
	static int functionKeyLeavesHtql(char*p, void* call_from, ReferData* tx); //&key_leaves_htql
		//<leaves_htql>
		//<leaves_htql>.<leave_htql> {name=:name; htql=:htql;}
	static int functionLeavesHtqlSchema(char* p, void* call_from, ReferData* tx); //&leaves_htql_schema(prefix)
		// prefix {} -- schema from <leave_htql> tags;

	static int functionUrlGo(char*p, void* call_from, ReferData* tx); //&url_go 
		//the new page
	static int functionGetEmail(char*p, void* call_from, ReferData* tx); //&get_email(fmt) 
		//get emails from text;
	static int functionTextWords(char*p, void* call_from, ReferData* tx); //&text_words(type, option) 
		//count words/get text from text/html;

	static int functionRepeatPatterns(char* p, void* call_from, ReferData* tx); //&repeat_patterns
		//analyze the most frequently appeared patterns in the text
		//<repeat_patterns>.<pattern>:length, repeat
		//<repeat_patterns>.<pattern> {words = <pattern_words>.<word>; positions=<positions>.<position>:tx }
	static int functioHtmlKeyText(char*p, void* call_from, ReferData* tx); //&html_key_text(key, key, ...)
		//get the main key value from a page
	static int findKeyText(char*p, PHyperBase* hyperbase, ReferLinkHeap* positions, ReferData* tx);

	static int functionOffset(char* p, void* call_from, ReferData* tx);	//&offset(start, len)
		//offset from the original text;
	static int functionPosition(char* p, void* call_from, ReferData* tx); //&position
		// <position>: from, to, lenght
	static int functionTagIndex(char* p, void* call_from, ReferData* tx); //&tag_index(source0/scope1, name)
		// <tag_index>:tag, name, index1, use_name
	static int functionSubStr(char* p, void* call_from, ReferData* tx); //&substr(position, offset)

	static int functionHtmlTitle(char*p, void* call_from, ReferData* tx); //&html_title
		//get the main title from a page
	static int functionHtmlMainDate(char*p, void* call_from, ReferData* tx); //&html_main_date
		//get the main date from a page
	static int functionHtmlMainText(char*p, void* call_from, ReferData* tx); //&html_main_text
		//get the main text from a page

	static int addHtmlQLFunctions(HTQLParser* parser);

	HtmlQLParser();
	virtual ~HtmlQLParser();
	virtual void reset();

protected:
	static int constructNameFromHyperText(const char* text, long max_len, ReferData* name, int max_words=1);
	static int findTagHtql(HTQLScope* lpElementCollection, HTQLScope* lpSelElement, ReferLinkHeap* results, char* suffix, int EnclosedSelItem=true, int HeuristicMethod=0, int* mark=0, int mark_level=0);
	static int findTagHtqlChild2Parent(HTQLScope* lpElementCollection, HTQLScope* lpSelElement, ReferLinkHeap* results, char* suffix, int level, int HeuristicMethod=0);
				//original find_htql algorithm, searching recursively from child to parent
	static int findTagHtqlParent2Child(HTQLScope* lpElementCollection, HTQLScope* lpSelElement, ReferLinkHeap* results, char* suffix, int HeuristicMethod=0, int* mark=0, int mark_level=0);
				//new algorithm searching with a linear parent-child relationship
	static int searchTagHtqlFromParentChildBits(ReferLinkHeap* results, FindTagHtqlStruct**index_map, int depth, char* suffix);
	static int get_tagChildIndex(HTQLScope* parent, HTQLScope* child, const char* attrname=0, const char* attrvalue=0); //use attrname is attrname is not null
	static int matchItemKeys(char* Source, char* Keys);
	static int findItemInfor(char* Source, int* Depth, int* FormatDepth, int* SecondFormatDept, int* BadDepth, int*GoodDepth, int*RecurDepth, int* maxidx);
	static HTQLScope* get_parentTag(HTQLScope* element, char* parent_name=0);
	static int get_Title(const char* page, ReferData* title, int level=0);
	static long get_tagName(HTQLTag* element, ReferData* tag_name);
	static long get_tagAttribute(HTQLTag* element, ReferData* attr_name, ReferData* attr_value);
	static HTQLScope* get_nextChildTag(HTQLScope* parent_tag, HTQLScope* tag, char* tag_name=0);
	static int tag_contain(HTQLScope* parent_tag, HTQLScope* tag);
	static int searchPlainPairScope(ReferData* data, HTQLScope** tag_scope);
	static int searchPlainPairScopePreviousTag(ReferData* data, long pos, HTQLScope* PreviousTag);
	static int searchPlainPairScopePreviousSameTag(HTQLScope* tag_scope);
	static int constructFindViewResult(HTQL* ql, ReferLinkHeap* path_htqls, ReferData* tx, int show_view_num=1, int maximum_views=10);
	static double computeTuplesSimilarity(HTQL* ql, int field_index1, long* tuples_score);
	static double computeNullTuplesSimilarity(HTQL* ql, long** fieldnullcounts=0);
	static long computeNullFieldCounts(HTQL* ql, long** fieldnullcounts);
	static int constructTagDataSyntaxStr(char* p, char* syntax_buf, int max_len, long* data_score);
	static int findDistingishedPlainData(char* p, ReferLinkHeap* results, int min_data_len=128);
					// find the data pieces that have length greater than min_data_len;
	static int findBestViewTagPos(char* htql, ReferData* data, int* max_pos, long* max, int* se_max_pos, long* se_max, double* similarity=0, long* tuple_score=0);
					// max_pos and se_max_pos is the position that maximized the view;
					// max and se_max is the tuples that can be got by the positions.
					// similarity is the gaps inserted in pairwise alignment in best view
					// tuple_score is the data length of the best view
	static int appendHtqlLeaveViewSchema(ReferData* htql_head, ReferData* htql_tail, HTQL* ql, ReferData* view_htql, int* cols=0);
					// htql_head.htql_tail is the original htql expression
					// returns the columns of the created htql, also returns in *cols
	static int createHtqlTailView(char* htql, ReferData* data, int max_pos, int se_max_pos, ReferData* view_htql, int* cols, long* rows);
					// rows and cols are the minimal rows and cols expected.
					// rows and rows return the rows and cols return from the view_htql
					// view_htql returns the created htql expression
	static int rankPathHtqlsByViewItems(HTQL* ql, ReferLinkHeap* path_htqls, ReferLinkHeap* view_htqls, int max_htql_num=10); 
					//view_htql: Key -- htql
					//view_htql: Value -- "max_pos:max:se_max_pos:se_max"
					//view_htql: Data -- rank
public:
	static int rankPathHtqlsToViews(HTQL* ql, ReferLinkHeap* path_htqls, ReferLinkHeap* view_htqls, int show_view_num=1, int maximum_views=10);
	static int filterHtqlResults(ReferLinkHeap* Source, ReferLinkHeap* Result, int HeuristicMethod, char* MatchString);
	static int evaluateHtqlExpr(HTQL* htql, const char* expr, ReferData* eval_result);
	static int dumpParentChildLinear(HTQLScope* tag, const char* filename);
};

#include "htmlql.h"

#endif
