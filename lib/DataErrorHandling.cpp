#include "Pcheader.h"

#include <tpublic/DataErrorHandling.h>

namespace tpublic
{

	namespace DataErrorHandling
	{

		std::vector<ErrorCallback> g_errorCallbackStack;

		void		
		PushErrorCallback(
			ErrorCallback					aErrorCallback)
		{
			g_errorCallbackStack.push_back(aErrorCallback);
		}

		void		
		PopErrorCallback()
		{
			g_errorCallbackStack.pop_back();
		}
		
		void		
		VerificationError(
			const std::optional<DebugInfo>& aDebugInfo,
			const char*						aFormat,
			...)
		{
			if(g_errorCallbackStack.size() > 0)
			{
				ErrorCallback& errorCallback = g_errorCallbackStack[g_errorCallbackStack.size() - 1];

				char buffer1[1024];
				TP_STRING_FORMAT_VARARGS(buffer1, sizeof(buffer1), aFormat);

				char buffer2[1024];
				if (aDebugInfo.has_value())
				{
					TP_STRING_FORMAT(buffer2, sizeof(buffer2), "[%s:%u] %s", aDebugInfo.value().m_file.c_str(), aDebugInfo.value().m_line, buffer1);
					errorCallback(buffer2);
				}
				else
				{
					errorCallback(buffer1);
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
			if(g_errorCallbackStack.size() > 0)
			{
				ErrorCallback& errorCallback = g_errorCallbackStack[g_errorCallbackStack.size() - 1];

				char buffer[1024];
				TP_STRING_FORMAT_VARARGS(buffer, sizeof(buffer), aFormat);
				errorCallback(buffer);
			}

			exit(EXIT_FAILURE);				
		}

	}

}