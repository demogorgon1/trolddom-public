#pragma once

namespace tpublic
{

	namespace DataErrorHandling
	{

		struct DebugInfo
		{
			std::string					m_file;
			uint32_t					m_line;
		};

		typedef std::function<void(const char*)> ErrorCallback;

		extern ErrorCallback g_errorCallback;

		void		SetErrorCallback(
						ErrorCallback					aErrorCallback);
		void		VerificationError(
						const std::optional<DebugInfo>&	aDebugInfo,
						const char*						aFormat,
						...);
		bool		VerifyStringId(
						const std::string&				aStringId);
		void		FatalError(
						const char*						aFormat,
						...);

	}

}

#define TP_VERIFY(_Condition, _DebugInfo, ...)																						\
	do																																\
	{																																\
		if(!(_Condition))																											\
			tpublic::DataErrorHandling::VerificationError(_DebugInfo, "" __VA_ARGS__);											\
	} while(false)

#define TP_VERIFY_STRING_ID(_StringId, _DebugInfo)																					\
	do																																\
	{																																\
		if(!tpublic::DataErrorHandling::VerifyStringId(_StringId))																\
			tpublic::DataErrorHandling::VerificationError(_DebugInfo, "Invalid string identifier: '%s'", _StringId.c_str());	\
	} while(false)
	
#define TP_CHECK(_Condition, ...)																									\
	do																																\
	{																																\
		if(!(_Condition))																											\
			tpublic::DataErrorHandling::FatalError("" __VA_ARGS__);																\
	} while(false)
