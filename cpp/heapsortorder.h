#ifndef CLY_HEAP_SORT_ORDER_H_20030110
#define CLY_HEAP_SORT_ORDER_H_20030110

typedef enum{ 
		SORT_ORDER_KEY_STR_INC, 
		SORT_ORDER_KEY_STR_DEC, 
		SORT_ORDER_VAL_STR_INC, 
		SORT_ORDER_VAL_STR_DEC, 
		SORT_ORDER_NUM_INC,
		SORT_ORDER_NUM_DEC,
		SORT_ORDER_KEY_NUM_INC, 
		SORT_ORDER_KEY_NUM_DEC, 
		SORT_ORDER_VAL_NUM_INC, 
		SORT_ORDER_VAL_NUM_DEC, 
		SORT_ORDER_KEY_VAL_STR_INC,
		SORT_ORDER_KEY_VAL_STR_DEC,
		SORT_ORDER_KEY_VAL_NUM_INC,
		SORT_ORDER_KEY_VAL_NUM_DEC,
		SORT_ORDER_KEYL_VALL_NUM_INC,
		SORT_ORDER_KEYL_VALL_NUM_DEC,
	} HeapSortOrder;


#endif