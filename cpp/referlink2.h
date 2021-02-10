#ifndef REFER_LINK2_H
#define REFER_LINK2_H

#include <stddef.h> 
#include "referlink.h"

class ReferLink2: public ReferLink{
public:
	ReferLink2* Prev;
	
	ReferLink2* insert(ReferLink2*p=0); //insert before this (or at the tail)
	int remove(ReferLink2*p);

	ReferLink2();
	~ReferLink2();
	void reset();
};


#endif
