#pragma once

#ifdef HAVE_WAFFLE_CONFIG_H
#include "WaffleConfig.h"
#endif

#ifdef VLD_FOUND
#include <vld.h>
#else // VLD_FOUND
#if defined(_DEBUG) && defined(WIN32)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
class MemLeakDetector
{
public:
	MemLeakDetector()
	{
		_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	}
	~MemLeakDetector()
	{
	}
} detector;
#endif

#endif // VLD_FOUND




