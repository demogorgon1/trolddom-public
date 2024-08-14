#pragma once

#include "../CharacterStat.h"
#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Achievement
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ACHIEVEMENT;
			static const bool TAGGED = false;

			struct StatTrigger
			{
				StatTrigger()
				{

				}

				StatTrigger(
					const SourceNode*	aSource)
				{
					m_id = CharacterStat::StringToId(aSource->m_name.c_str());
					TP_VERIFY(m_id != CharacterStat::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid character stat.", aSource->m_name.c_str());
					m_threshold = aSource->GetUInt32();
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_id);
					aWriter->WriteUInt(m_threshold);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_id))
						return false;
					if(!aReader->ReadUInt(m_threshold))
						return false;
					return true;
				}

				// Public data
				CharacterStat::Id	m_id = CharacterStat::INVALID_ID;
				uint32_t			m_threshold = 0;
			};

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
				m_sortKey = aSource->GetSortKey();

				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if (aChild->m_name == "description")
							m_description = aChild->GetString();
						else if (aChild->m_name == "points")
							m_points = aChild->GetUInt32();
						else if (aChild->m_name == "priority")
							m_priority = aChild->GetUInt32();
						else if (aChild->m_name == "no_progress_values")
							m_noProgressValues = aChild->GetBool();
						else if (aChild->m_name == "category")
							m_categoryId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ACHIEVEMENT_CATEGORY, aChild->GetIdentifier());
						else if (aChild->m_name == "root")
							m_rootId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ACHIEVEMENT, aChild->GetIdentifier());
						else if (aChild->m_name == "icon")
							m_iconSpriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
						else if (aChild->m_tag == "stat_trigger")
							m_statTrigger = StatTrigger(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteString(m_string);
				aStream->WriteString(m_description);
				aStream->WriteUInt(m_points);
				aStream->WriteUInt(m_priority);
				aStream->WriteUInt(m_categoryId);
				aStream->WriteUInt(m_rootId);
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteOptionalObject(m_statTrigger);
				aStream->WriteUInt(m_sortKey);
				aStream->WriteBool(m_noProgressValues);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadString(m_description))
					return false;
				if (!aStream->ReadUInt(m_points))
					return false;
				if (!aStream->ReadUInt(m_priority))
					return false;
				if (!aStream->ReadUInt(m_categoryId))
					return false;
				if (!aStream->ReadUInt(m_rootId))
					return false;
				if (!aStream->ReadUInt(m_iconSpriteId))
					return false;
				if(!aStream->ReadOptionalObject(m_statTrigger))
					return false;
				if (!aStream->ReadUInt(m_sortKey))
					return false;
				if (!aStream->ReadBool(m_noProgressValues))
					return false;
				return true;
			}

			// Public data
			std::string						m_string;
			std::string						m_description;
			uint32_t						m_points = 10;
			uint32_t						m_priority = 0;
			uint32_t						m_categoryId = 0;
			uint32_t						m_rootId = 0;
			uint32_t						m_iconSpriteId = 0;
			std::optional<StatTrigger>		m_statTrigger;
			uint64_t						m_sortKey = 0;
			bool							m_noProgressValues = false;
		};

	}

}