/* @(#)debug.h	1.2 */

#ifdef OSDEBUG
#define ASSERT(EX) if (EX) ; else assfail("EX", __FILE__, __LINE__)
#else
#define ASSERT(EX)
#endif

#define PFLOGON 0
#define PHASHLOGON 0
#define SWLOGON 0
#define LOCKLOGON 0
#define SPTLOGON 0
