#pragma once

#include "IReader.h"
#include "ItemInstance.h"
#include "IWriter.h"

namespace tpublic
{

	class Mail
	{
	public:
		static const uint32_t FORMAT_VERSION = 1;

		void
		ToStream(
			IWriter*		aWriter) const
		{
			aWriter->WriteUInt(FORMAT_VERSION);
			aWriter->WriteUInt(m_toCharacterId);
			aWriter->WriteUInt(m_fromCharacterId);
			aWriter->WriteString(m_fromName);
			aWriter->WriteObjects(m_items);
			aWriter->WriteInt(m_cash);
			aWriter->WriteString(m_subject);
			aWriter->WriteString(m_body);
		}

		bool
		FromStream(
			IReader*		aReader)
		{
			uint32_t formatVersion;
			if(!aReader->ReadUInt(formatVersion))
				return false;
			if(formatVersion != FORMAT_VERSION)
				return false;

			if (!aReader->ReadUInt(m_toCharacterId))
				return false;
			if (!aReader->ReadUInt(m_fromCharacterId))
				return false;
			if(!aReader->ReadString(m_fromName))
				return false;
			if(!aReader->ReadObjects(m_items))
				return false;
			if (!aReader->ReadInt(m_cash))
				return false;
			if (!aReader->ReadString(m_subject))
				return false;
			if (!aReader->ReadString(m_body))
				return false;
			return true;
		}

		// Public data
		uint32_t							m_toCharacterId = 0;
		uint32_t							m_fromCharacterId = 0;
		std::string							m_fromName;
		std::vector<ItemInstance>			m_items;
		int64_t								m_cash = 0;
		std::string							m_subject;
		std::string							m_body;
	};

}