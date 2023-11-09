#pragma once

#include "DataErrorHandling.h"
#include "SourceNode.h"

namespace tpublic
{

	class Macro
	{
	public:
		Macro(
			const char*							aName,
			const DataErrorHandling::DebugInfo&	aDebugInfo);
		~Macro();

		// Public data
		DataErrorHandling::DebugInfo	m_debugInfo;
		std::string						m_name;
		std::unique_ptr<SourceNode>		m_body;
	};

}