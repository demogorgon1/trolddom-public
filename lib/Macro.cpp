#include "Pcheader.h"

#include <tpublic/Macro.h>

namespace tpublic
{

	Macro::Macro(
		const char*							aName,
		const DataErrorHandling::DebugInfo& aDebugInfo)
		: m_name(aName)
		, m_debugInfo(aDebugInfo)
	{

	}
	
	Macro::~Macro()
	{

	}
	
}