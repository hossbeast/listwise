#ifndef _CONTROL_H
#define _CONTROL_H

#define fail(fmt, ...)							\
	do {															\
		printf(fmt " at [%s:%d (%s)]\n"	\
			, ##__VA_ARGS__								\
			, __FILE__										\
			, __LINE__										\
			, __FUNCTION__								\
		);															\
		return 0;												\
	} while(0)
 
#define fatal(x, ...)												\
	do {																			\
		if((x(__VA_ARGS__)) == 0)								\
		{																				\
			printf(#x " failed at [%s:%d (%s)]\n"	\
				, __FILE__													\
				, __LINE__													\
				, __FUNCTION__											\
			);																		\
			return 0;															\
		}																				\
	} while(0)

#define fatal_os(x, ...)																		\
	do {																											\
		if((x(__VA_ARGS__)) != 0)																\
		{																												\
			printf(#x " failed with: [%d][%s] at [%s:%d (%s)]\n"	\
				, errno																							\
				, strerror(errno)																		\
				, __FILE__																					\
				, __LINE__																					\
				, __FUNCTION__																			\
			);																										\
			return 0;																							\
		}																												\
	} while(0)

#endif
