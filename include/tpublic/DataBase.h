#pragma once

#include "Base64.h"
#include "DataErrorHandling.h"
#include "DataType.h"
#include "Helpers.h"
#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"
#include "VectorIO.h"

namespace tpublic
{

	class Manifest;
	
	struct DataBase
	{
		typedef std::unordered_map<std::string, std::string> PropertyTable;

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

		virtual
		~DataBase()
		{

		}

		void
		VerifyBase() const
		{
			TP_VERIFY_STRING_ID(m_name, m_debugInfo);
			TP_VERIFY(m_defined, m_debugInfo, "'%s' not defined.", m_name.c_str());
			TP_VERIFY(m_id != 0, m_debugInfo, "'%s' has no id.", m_name.c_str());
		}

		bool
		FromSourceBase(
			const SourceNode*					aSource)
		{
			if(aSource->m_name == "tags")
			{	
				aSource->GetIdArray(DataType::ID_TAG, m_tagIds);
				return true;
			}
			else if(aSource->m_name == "binary")
			{
				std::vector<uint8_t> binary;
				Base64::Decode(aSource->GetString(), binary);
				VectorIO::Reader reader(binary);
				bool ok = FromStream(&reader);
				TP_VERIFY(ok, aSource->m_debugInfo, "Failed to read binary.");
				return true;
			}
			return false;
		}

		void
		ToStreamBase(
			IWriter*							aStream) const
		{
			assert(m_defined);

			aStream->WriteString(m_name);
			aStream->WriteUInt(m_id);
			aStream->WriteUInts(m_tagIds);
		}

		bool
		FromStreamBase(
			IReader*							aStream)
		{
			if(!aStream->ReadString(m_name))
				return false;
			if(!aStream->ReadUInt(m_id))
				return false;
			if (!aStream->ReadUInts(m_tagIds))
				return false;

			m_defined = true;
			return true;
		}

		bool
		HasTag(
			uint32_t							aTagId) const
		{
			for(uint32_t t : m_tagIds)
			{
				if(t == aTagId)
					return true;
			}
			return false;
		}

		void
		ToPropertyTableBase(
			PropertyTable&						aOut) const
		{
			aOut["name"] = m_name;
			aOut["id"] = Helpers::Format("%u", m_id);

			if(m_tagIds.size() > 0)
			{
				std::string tagString;
				for(uint32_t tagId : m_tagIds)
					tagString += Helpers::Format("%s%u", tagString.empty() ? "" : " ", tagId);
				aOut["tags"] = tagString;
			}
		}

		// Virtual interface
		virtual void	FromSource(
							const SourceNode*	aNode) = 0;
		virtual void	ToStream(
							IWriter*			aStream) const = 0;
		virtual bool	FromStream(
							IReader*			aStream) = 0;	

		// Virtual methods
		virtual void	PrepareRuntime(
							uint8_t				/*aRuntime*/,
							const Manifest*		/*aManifest*/) { }
		virtual void	ToPropertyTable(
							PropertyTable&		aOut) const { ToPropertyTableBase(aOut); }

		// Public data
		std::string									m_name;
		uint32_t									m_id;
		std::vector<uint32_t>						m_tagIds;
		bool										m_defined;
		std::optional<DataErrorHandling::DebugInfo>	m_debugInfo;
		const ComponentManager*						m_componentManager;
	};

}