#pragma once

#include "../DataBase.h"
#include "../Requirement.h"

namespace tpublic
{

	namespace Data
	{

		struct Zone
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ZONE;
			static const bool TAGGED = true;

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
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if(aChild->m_name == "town")
							m_town = aChild->GetBool();
						else if (aChild->m_name == "no_map")
							m_noMap = aChild->GetBool();
						else if (aChild->m_name == "can_query_position")
							m_canQueryPosition = aChild->GetBool();
						else if (aChild->m_name == "fishing_loot_table")
							m_fishingLootTableId = aChild->GetId(DataType::ID_LOOT_TABLE);
						else if (aChild->m_name == "fishing_trigger_ability")
							m_fishingTriggerAbilityId = aChild->GetId(DataType::ID_ABILITY);
						else if (aChild->m_name == "fishing_trigger_ability_chance")
							m_fishingTriggerAbilityChance = aChild->GetUInt32();
						else if (aChild->m_name == "player_trigger_ability")
							m_playerTriggerAbilityId = aChild->GetId(DataType::ID_ABILITY);
						else if (aChild->m_name == "player_trigger_ability_chance")
							m_playerTriggerAbilityChance = aChild->GetUInt32();
						else if (aChild->m_tag == "player_trigger_ability_requirement")
							m_playerTriggerAbilityRequirements.push_back(Requirement(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteString(m_string);
				aWriter->WriteBool(m_town);
				aWriter->WriteBool(m_noMap);
				aWriter->WriteBool(m_canQueryPosition);
				aWriter->WriteUInt(m_fishingLootTableId);
				aWriter->WriteUInt(m_fishingTriggerAbilityId);
				aWriter->WriteUInt(m_fishingTriggerAbilityChance);
				aWriter->WriteUInt(m_playerTriggerAbilityId);
				aWriter->WriteObjects(m_playerTriggerAbilityRequirements);
				aWriter->WriteUInt(m_playerTriggerAbilityChance);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadString(m_string))
					return false;
				if(!aReader->ReadBool(m_town))
					return false;
				if (!aReader->ReadBool(m_noMap))
					return false;
				if (!aReader->ReadBool(m_canQueryPosition))
					return false;
				if (!aReader->ReadUInt(m_fishingLootTableId))
					return false;
				if (!aReader->ReadUInt(m_fishingTriggerAbilityId))
					return false;
				if (!aReader->ReadUInt(m_fishingTriggerAbilityChance))
					return false;
				if (!aReader->ReadUInt(m_playerTriggerAbilityId))
					return false;
				if(!aReader->ReadObjects(m_playerTriggerAbilityRequirements))
					return false;
				if (!aReader->ReadUInt(m_playerTriggerAbilityChance))
					return false;
				return true;
			}

			void
			ToPropertyTable(
				PropertyTable&			aOut) const override
			{
				ToPropertyTableBase(aOut);
				aOut["string"] = m_string;
			}

			// Public data
			std::string					m_string;
			bool						m_town = false;			
			bool						m_noMap = false;
			bool						m_canQueryPosition = false;
			
			uint32_t					m_fishingLootTableId = 0;
			uint32_t					m_fishingTriggerAbilityId = 0;
			uint32_t					m_fishingTriggerAbilityChance = 0;

			uint32_t					m_playerTriggerAbilityId = 0;
			std::vector<Requirement>	m_playerTriggerAbilityRequirements;
			uint32_t					m_playerTriggerAbilityChance = 0;
		};

	}

}