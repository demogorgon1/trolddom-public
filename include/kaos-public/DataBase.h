#pragma once

#include "DataErrorHandling.h"
#include "DataType.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace kaos_public
{

	struct DataBase
	{
		DataBase()
			: m_defined(false)
			, m_id(0)
		{

		}

		void
		VerifyBase() const
		{
			KP_VERIFY_STRING_ID(m_name, m_debugInfo);
			KP_VERIFY(m_defined, m_debugInfo, "'%s' not defined.", m_name.c_str());
			KP_VERIFY(m_id != 0, m_debugInfo, "'%s' has no id.", m_name.c_str());
		}

		void
		ToStreamBase(
			IWriter*							aStream) const
		{
			assert(m_defined);

			aStream->WriteString(m_name);
			aStream->WriteUInt(m_id);
		}

		bool
		FromStreamBase(
			IReader*							aStream)
		{
			if(!aStream->ReadString(m_name))
				return false;
			if(!aStream->ReadUInt(m_id))
				return false;

			m_defined = true;
			return true;
		}

		// Virtual interface
		virtual void	FromSource(
							const Parser::Node* aNode) = 0;
		virtual void	ToStream(
							IWriter*			aStream) const = 0;
		virtual bool	FromStream(
							IReader*			aStream) = 0;							

		// Public data
		std::string									m_name;
		uint32_t									m_id;
		bool										m_defined;
		std::optional<DataErrorHandling::DebugInfo>	m_debugInfo;
	};

}