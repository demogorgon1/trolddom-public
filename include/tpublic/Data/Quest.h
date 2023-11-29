#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Quest
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_QUEST;

			enum Type : uint8_t
			{
				INVALID_TYPE,

				TYPE_NORMAL,
				TYPE_ELITE,
				TYPE_DUNGEON
			};

			static Type
			SourceToType(
				const SourceNode*		aSource)
			{
				std::string_view t(aSource->GetIdentifier());
				if(t == "normal")
					return TYPE_NORMAL;
				else if(t == "elite")
					return TYPE_ELITE;
				else if(t == "dungeon")
					return TYPE_DUNGEON;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid type.", aSource->GetIdentifier());
				return INVALID_TYPE;
			}

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				auto appendString = [&](
					std::string& aString,
					const char*  aAppend)
				{
					size_t length = aString.length();
					if(length > 0 && aString[length - 1] != '\n')
						aString.push_back(' ');
					aString.append(aAppend);
				};				

				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "string")
						m_string = aChild->GetString();
					else if (aChild->m_name == "text")
						aChild->GetArray()->ForEachChild([&](const SourceNode* aLine) { appendString(m_text, aLine->GetString()); });
					else if (aChild->m_name == "description")
						aChild->GetArray()->ForEachChild([&](const SourceNode* aLine) { appendString(m_description, aLine->GetString()); });
					else if (aChild->m_name == "progress")
						aChild->GetArray()->ForEachChild([&](const SourceNode* aLine) { appendString(m_progress, aLine->GetString()); });
					else if(aChild->m_name == "level")
						m_level = aChild->GetUInt32();
					else if(aChild->m_name == "type")
						m_type = SourceToType(aChild);
					else if (aChild->m_name == "objectives")
						aChild->GetIdArray(DataType::ID_OBJECTIVE, m_objectives);
					else if (aChild->m_name == "reward_one_item")
						aChild->GetIdArray(DataType::ID_ITEM, m_rewardOneItem);
					else if (aChild->m_name == "reward_all_items")
						aChild->GetIdArray(DataType::ID_ITEM, m_rewardAllItems);
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);

				aStream->WriteString(m_string);
				aStream->WriteString(m_text);
				aStream->WriteString(m_description);
				aStream->WriteString(m_progress);
				aStream->WriteUInts(m_objectives);
				aStream->WriteUInt(m_level);
				aStream->WritePOD(m_type);
				aStream->WriteUInts(m_rewardOneItem);
				aStream->WriteUInts(m_rewardAllItems);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;

				if (!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadString(m_text, 16 * 1024))
					return false;
				if (!aStream->ReadString(m_description, 16 * 1024))
					return false;
				if (!aStream->ReadString(m_progress, 16 * 1024))
					return false;
				if(!aStream->ReadUInts(m_objectives))
					return false;
				if(!aStream->ReadUInt(m_level))
					return false;
				if(!aStream->ReadPOD(m_type))
					return false;
				if (!aStream->ReadUInts(m_rewardOneItem))
					return false;
				if (!aStream->ReadUInts(m_rewardAllItems))
					return false;
				return true;
			}

			// Public data
			std::string				m_string;
			std::string				m_text;
			std::string				m_description;
			std::string				m_progress;
			std::vector<uint32_t>	m_objectives;
			uint32_t				m_level = 1;
			Type					m_type = TYPE_NORMAL;
			std::vector<uint32_t>	m_rewardOneItem;
			std::vector<uint32_t>	m_rewardAllItems;
		};

	}

}