#ifndef CLY_HTMLQL_H_20020815
#define CLY_HTMLQL_H_20020815

#include "htql.h"

#define LEAVE_SINGLE_ITEM		0x01
#define LEAVE_ENCLOSED_ITEM		0x02

class HtmlQL: public HTQL {
public:
	enum{ heuBEST, heuDEPTH_SHORT, heuDEPTH_LONG, heuBEST_LEAVE, heuBEST_VIEW, heuNO_CLASS };
	int setPageMark(int* pagemark, int marklevel); 

	HtmlQL();
	~HtmlQL();
};

#endif
