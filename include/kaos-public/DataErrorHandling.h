#pragma once

namespace kaos_public
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

#define KP_VERIFY(_Condition, _DebugInfo, ...)																						\
	do																																\
	{																																\
		if(!(_Condition))																											\
			kaos_public::DataErrorHandling::VerificationError(_DebugInfo, "" __VA_ARGS__);											\
	} while(false)

#define KP_VERIFY_STRING_ID(_StringId, _DebugInfo)																					\
	do																																\
	{																																\
		if(!kaos_public::DataErrorHandling::VerifyStringId(_StringId))																\
			kaos_public::DataErrorHandling::VerificationError(_DebugInfo, "Invalid string identifier: '%s'", _StringId.c_str());	\
	} while(false)
	
#define KP_CHECK(_Condition, ...)																									\
	do																																\
	{																																\
		if(!(_Condition))																											\
			kaos_public::DataErrorHandling::FatalError("" __VA_ARGS__);																\
	} while(false)
