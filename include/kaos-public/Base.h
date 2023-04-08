#pragma once

#include <filesystem>
#include <functional>
#include <optional>

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>

#if defined(WIN32)

	#define KP_STRING_FORMAT_VARARGS(Buffer, BufferSize, Format)						\
		{																				\
			va_list _list;																\
			va_start(_list, Format);													\
			int n = vsnprintf_s(Buffer, BufferSize, _TRUNCATE, Format, _list);			\
			if(n < 0)																	\
				Buffer[0] = '\0';														\
			va_end(_list);																\
		}

	#define KP_STRING_FORMAT(Buffer, BufferSize, Format, ...)							\
		{																				\
			sprintf_s(Buffer, BufferSize, Format, __VA_ARGS__);							\
		}

#else

	#define KP_STRING_FORMAT_VARARGS(Buffer, BufferSize, Format)						\
		{																				\
			va_list _list;																\
			va_start(_list, Format);													\
			int n = vsnprintf(Buffer, BufferSize, Format, _list);						\
			if(n < 0)																	\
				Buffer[0] = '\0';														\
			va_end(_list);																\
		}

	#define KP_STRING_FORMAT(Buffer, BufferSize, Format, ...)							\
		{																				\
			snprintf(Buffer, BufferSize, Format, __VA_ARGS__);							\
		}

#endif

