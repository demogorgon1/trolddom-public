#include "Pcheader.h"

#include <tpublic/Data/Item.h>

#include <tpublic/Helpers.h>
#include <tpublic/Mail.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	void
	Mail::ToStream(
		IWriter* aWriter) const
	{
		aWriter->WriteUInt(FORMAT_VERSION);
		aWriter->WriteUInt(m_toCharacterId);
		aWriter->WriteUInt(m_fromCharacterId);
		aWriter->WriteString(m_fromName);
		aWriter->WriteObjects(m_items);
		aWriter->WriteInt(m_cash);
		aWriter->WriteString(m_subject);
		aWriter->WriteString(m_body);
		aWriter->WritePOD(m_flags); // Version 2
	}

	bool
	Mail::FromStream(
		IReader* aReader)
	{
		uint32_t formatVersion;
		if (!aReader->ReadUInt(formatVersion))
			return false;
		if (formatVersion > FORMAT_VERSION)
			return false;

		if (!aReader->ReadUInt(m_toCharacterId))
			return false;
		if (!aReader->ReadUInt(m_fromCharacterId))
			return false;
		if (!aReader->ReadString(m_fromName))
			return false;
		if (!aReader->ReadObjects(m_items))
			return false;
		if (!aReader->ReadInt(m_cash))
			return false;
		if (!aReader->ReadString(m_subject))
			return false;
		if (!aReader->ReadString(m_body))
			return false;

		if (formatVersion >= 2)
		{
			if (!aReader->ReadPOD(m_flags))
				return false;
		}
		return true;
	}

	void
	Mail::SetAutoDelete()
	{
		m_flags |= FLAG_AUTO_DELETE;
	}

	bool
	Mail::ShouldAutoDelete() const
	{
		return m_flags & FLAG_AUTO_DELETE;
	}

	void		
	Mail::FromString(
		const char*		aString,
		const Manifest* aManifest,
		uint32_t		aOverrideToCharacterId)
	{
		struct Part
		{
			std::string		m_name;
			std::string		m_value;
		};

		std::vector<Part> parts;

		{
			std::stringstream tokenizer(aString);
			std::string token;
			while (std::getline(tokenizer, token, ';'))
			{
				Helpers::TrimString(token);
				std::stringstream tokenizer2(token.c_str());
				std::string token2;
				std::vector<std::string> nameAndValue;
				while (std::getline(tokenizer2, token2, '='))
				{
					Helpers::TrimString(token2);
					nameAndValue.push_back(token2);
				}

				TP_CHECK(nameAndValue.size() == 2, "Syntax error.");
				parts.push_back({ nameAndValue[0], nameAndValue[1] });
			}
		}
	
		for(const Part& part : parts)
		{
			if(part.m_name == "to")
			{
				m_toCharacterId = (uint32_t)strtoul(part.m_value.c_str(), NULL, 10);
				TP_CHECK(m_toCharacterId, "Invalid character id: %s.", part.m_name.c_str());
			}
			else if(part.m_name == "from")
			{
				m_fromName = part.m_value;
			}
			else if(part.m_name == "cash")
			{
				m_cash = (int64_t)strtoul(part.m_value.c_str(), NULL, 10);
			}
			else if (part.m_name == "subj")
			{
				m_subject = part.m_value;
			}
			else if (part.m_name == "body")
			{
				m_body = part.m_value;
			}
			else if (part.m_name == "item")
			{
				uint32_t itemId = aManifest->TryGetExistingIdByName<Data::Item>(part.m_value.c_str());
				TP_CHECK(itemId != 0, "Invalid item name: %s.", part.m_value.c_str());
				m_items.push_back({ itemId });
			}
			else
			{
				TP_CHECK(false, "Invalid field: %s.", part.m_name.c_str());
			}
		}

		if(aOverrideToCharacterId != 0)
			m_toCharacterId = aOverrideToCharacterId;
	}

	std::string	
	Mail::GetItemString(
		const Manifest* aManifest) const
	{
		std::string t;

		for(size_t i = 0; i < m_items.size(); i++)
		{
			const ItemInstance& itemInstance = m_items[i]; 

			if(i > 0)			
				t += ",";
			
			const Data::Item* item = aManifest->GetById<Data::Item>(itemInstance.m_itemId);
			t += item->m_string;

			if(itemInstance.m_quantity > 1)
				t += Helpers::Format("(%u)", itemInstance.m_quantity);
		}

		return t;
	}

	void		
	Mail::DebugPrint(
		const Manifest* aManifest) const
	{
		printf("to:%u;\n", m_toCharacterId);
		printf("from:%u;\n", m_fromCharacterId);
		printf("from_name:%s\n", m_fromName.c_str());
		printf("subj:%s\n", m_subject.c_str());
		printf("body:%s\n", m_body.c_str());
		printf("cash:%zd\n", m_cash);
		printf("items:%s\n", GetItemString(aManifest).c_str());
	}

}