#include "Pcheader.h"

#include <kpublic/DataErrorHandling.h>

namespace kpublic
{

	namespace DataErrorHandling
	{

		ErrorCallback g_errorCallback;

		void		
		SetErrorCallback(
			ErrorCallback					aErrorCallback)
		{
			g_errorCallback = aErrorCallback;
		}
		
		void		
		VerificationError(
			const std::optional<DebugInfo>& aDebugInfo,
			const char*						aFormat,
			...)
		{
			if(g_errorCallback)
			{
				char buffer1[1024];
				KP_STRING_FORMAT_VARARGS(buffer1, sizeof(buffer1), aFormat);

				char buffer2[1024];
				if (aDebugInfo.has_value())
				{
					KP_STRING_FORMAT(buffer2, sizeof(buffer2), "[%s:%u] %s", aDebugInfo.value().m_file.c_str(), aDebugInfo.value().m_line, buffer1);
					g_errorCallback(buffer2);
				}
				else
				{
					g_errorCallback(buffer1);
				}
			}

			exit(EXIT_FAILURE);				
		}

		bool		
		VerifyStringId(
			const std::string&				aStringId)
		{
			const char* p = aStringId.c_str();

			if(!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_'))
				return false;
				
			p++;

			while(*p != '\0')
			{
				if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') || *p == '_' || (*p >= '0' && *p <= '9')))
					return false;

				p++;
			}

			return true;
		}

		void		
		FatalError(
			const char*						aFormat,
			...)
		{
			if(g_errorCallback)
			{
				char buffer[1024];
				KP_STRING_FORMAT_VARARGS(buffer, sizeof(buffer), aFormat);
				g_errorCallback(buffer);
			}

			exit(EXIT_FAILURE);				
		}

	}

}