/* DEBUG macro ------------ 
 *
 * Q-ha Park
 */

#ifndef __DEBUG_H_

#ifdef DEBUG
	#include <assert.h>
    #define DPRINTF(fmt, args...) \
		printf(__FILE__":%d: " fmt,__LINE__, ## args)
#else
	#define NDEBUG
	#include <assert.h>
    #define DPRINTF(fmt, args...)
#endif


#endif 
