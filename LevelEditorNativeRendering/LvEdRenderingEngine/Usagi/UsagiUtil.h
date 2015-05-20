#pragma once

#include <windows.h>

namespace LvEdEngine{

	inline void GetUsagiDir( wchar_t out[], size_t size ) {
		GetEnvironmentVariable( L"USAGI_DIR", out, (DWORD)size );
	}

}