/***************************************

	Useful macros

***************************************/

#ifndef __DPMACROS_H__
#define __DPMACROS_H__

/***************************************

	Helpful worker macros

***************************************/

#define SAFE_DELETE(p) \
	{ \
		if (p) { \
			delete (p); \
			(p) = NULL; \
		} \
	}

#define SAFE_DELETE_ARRAY(p) \
	{ \
		if (p) { \
			delete[] (p); \
			(p) = NULL; \
		} \
	}

#define SAFE_RELEASE(p) \
	{ \
		if (p) { \
			(p)->Release(); \
			(p) = NULL; \
		} \
	}

#endif
