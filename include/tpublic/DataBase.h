#pragma once

#include "DataErrorHandling.h"
#include "DataType.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{

	class Manifest;
	
	struct DataBase
	{
		enum Runtime : uint8_t
		{
			RUNTIME_SERVER = 0x01,
			RUNTIME_CLIENT = 0x02
		};
		
		DataBase()
			: m_defined(false)
			, m_id(0)
			, m_componentManager(NULL)
		{

		}

		void
		VerifyBase() const
		{
			TP_VERIFY_STRING_ID(m_name, m_debugInfo);
			TP_VERIFY(m_defined, m_debugInfo, "'%s' not defined.", m_name.c_str());
			TP_VERIFY(m_id != 0, m_debugInfo, "'%s' has no id.", m_name.c_str());
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
							const SourceNode* aNode) = 0;
		virtual void	ToStream(
							IWriter*			aStream) const = 0;
		virtual bool	FromStream(
							IReader*			aStream) = 0;							

		// Virtual methods
		virtual void	PrepareRuntime(
							uint8_t				/*aRuntime*/,
							const Manifest*		/*aManifest*/) { }

		// Public data
		std::string									m_name;
		uint32_t									m_id;
		bool										m_defined;
		std::optional<DataErrorHandling::DebugInfo>	m_debugInfo;
		const ComponentManager*						m_componentManager;
	};

}