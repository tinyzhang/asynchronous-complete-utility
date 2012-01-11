/*
work flow common
author tinyzhang
date 2012/01/09
*/
#ifndef __workflowcommon_h__
#define __workflowcommon_h__

#include "workflowconf.h"
#include <cassert>
#include <iostream>

#undef ASYNC_UTILITY_ASSERT
#if ASYNC_UTILITY_DEBUG
#define ASYNC_UTILITY_ASSERT assert
#else
#define ASYNC_UTILITY_ASSERT ((void)0)
#endif

#undef ASYNC_UTILITY_DEBUG_OUTPUT
#if ASYNC_UTILITY_DEBUG
#define ASYNC_UTILITY_DEBUG_OUTPUT(info) std::cout << info << std::endl
#else
#define ASYNC_UTILITY_DEBUG_OUTPUT(info) ((void)0)
#endif

#undef DISALLOW_EVIL_CONSTRUCTORS
#define DISALLOW_EVIL_CONSTRUCTORS(TypeName) \
	TypeName( const TypeName& ); \
	void operator = ( const TypeName& )

#undef SAFE_DELETE
#define SAFE_DELETE(ptr) \
	if (ptr) \
	{ \
		delete ptr; \
		ptr = NULL; \
	}

#endif