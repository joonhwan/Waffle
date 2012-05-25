#include "wlogdebugoutput.h"
#include "wlogoutput.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


//virtual
void WLogDebugOutput::write(const WLogEvent& event)
{
	::OutputDebugStringW(reinterpret_cast<const WCHAR*>(event.logMessage().utf16()));
	::OutputDebugStringW(L"\n");
}

