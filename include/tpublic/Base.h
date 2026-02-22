#pragma once

#if defined(WIN32)
	#pragma warning(disable: 4702) // Unreachable code, triggered by /LTCG in release builds
#endif

#include <atomic>
#include <filesystem>
#include <functional>
#include <map>
#include <mutex>
#include <optional>
#include <random>
#include <set>
#include <thread>
#include <unordered_set>
#include <vector>

#include <assert.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <nwork/API.h>

#if defined(WIN32)

	#define TP_STRING_FORMAT_VARARGS(Buffer, BufferSize, Format)						\
		do																				\
		{																				\
			va_list _list;																\
			va_start(_list, Format);													\
			int n = vsnprintf_s(Buffer, BufferSize, _TRUNCATE, Format, _list);			\
			if(n < 0)																	\
				Buffer[0] = '\0';														\
			va_end(_list);																\
		} while(false)

	#define TP_STRING_FORMAT(Buffer, BufferSize, Format, ...)							\
		do																				\
		{																				\
			sprintf_s(Buffer, BufferSize, Format, __VA_ARGS__);							\
		} while(false)

#else

	#define TP_STRING_FORMAT_VARARGS(Buffer, BufferSize, Format)						\
		do																				\
		{																				\
			va_list _list;																\
			va_start(_list, Format);													\
			int n = vsnprintf(Buffer, BufferSize, Format, _list);						\
			if(n < 0)																	\
				Buffer[0] = '\0';														\
			va_end(_list);																\
		} while(false)

	#define TP_STRING_FORMAT(Buffer, BufferSize, Format, ...)							\
		do																				\
		{																				\
			snprintf(Buffer, BufferSize, Format, __VA_ARGS__);							\
		} while(false)

#endif

#define TP_UNUSED(_X) (void)_X;

namespace tpublic::Base
{

	inline uint32_t
	HashBuffer(
		const void*		aBuffer,
		size_t			aBufferSize)
	{
		const uint8_t* p = (const uint8_t*)aBuffer;
		uint32_t hash = 0x811c9dc5;

		for (size_t i = 0; i < aBufferSize; i++)
		{
			hash ^= (uint32_t)p[i];
			hash *= 0x01000193;
		}

		return hash;
	}

	// Can't use std::min and std::max because Windows farts up the global namespace with min() and max() macros

	template <typename _T>
	inline _T
	Min(
		_T				aA,
		_T				aB)
	{
		return aA < aB ? aA : aB;
	}

	template <typename _T>
	inline _T
	Max(
		_T				aA,
		_T				aB)
	{
		return aA > aB ? aA : aB;
	}

}
