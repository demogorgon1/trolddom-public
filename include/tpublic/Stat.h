#pragma once

#include "Hash.h"
#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	namespace Stat
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_DEXTERITY,
			ID_STRENGTH,
			ID_WISDOM,
			ID_CONSTITUTION,
			ID_SPIRIT,
			ID_ARMOR,
			ID_PHYSICAL_CRITICAL_STRIKE_CHANCE,
			ID_SPELL_CRITICICAL_STRIKE_CHANCE,
			ID_DODGE_CHANCE,
			ID_BLOCK_CHANCE,
			ID_PARRY_CHANCE,
			ID_BLOCK_VALUE,
			ID_ATTACK_HASTE,
			ID_SPELL_HASTE,
			ID_SPELL_DAMAGE,
			ID_HEALING,
			ID_HEALTH_PER_5_SECONDS,
			ID_MANA_PER_5_SECONDS,
			ID_ATTACK_POWER,
			ID_STEALTH,
			ID_RESILIENCE,
			ID_HIT,

			NUM_IDS
		};
		
		struct Info
		{
			const char*		m_name;
			const char*		m_shortName;
			const char*		m_longName;
			const char*		m_description;
			float			m_budgetCost;
			bool			m_percentage;
			bool			m_basic;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{			
			{ NULL, NULL, NULL, NULL, 0, false, false },

			{ "dexterity",				"DEX",			"Dexterity",								NULL,																	1.0f,	false,	true },
			{ "strength",				"STR",			"Strength",									NULL,																	1.0f,	false,	true },
			{ "wisdom",					"WIS",			"Wisdom",									NULL,																	1.0f,	false,	true },
			{ "constitution",			"CON",			"Constitution",								NULL,																	1.0f,	false,	true },
			{ "spirit",					"SPI",			"Spirit",									NULL,																	1.0f,	false,	true },
			{ "armor",					"ARM",			"Armor",									NULL,																	1.0f,	false,	false },
			{ "phys_crit_chance",		"CRIT",			"Critical Strike Chance (Physical)",		"Improves your chance to get a critical strike by %u%%.",				10.0f,	true,	false },
			{ "spell_crit_chance",		"SPELLCRIT",	"Critical Strike Chance (Spells)",			"Improves your chance to get a critical strike with spells by %u%%.",	10.0f,	true,	false },
			{ "dodge_chance",			"DODGE",		"Dodge Chance",								"Improves your chance to dodge an attack by %u%%.",						10.0f,	true,	false },
			{ "block_chance",			"BLOCK",		"Block Chance",								"Improves your chance to block an attack by %u%%.",						3.0f,	true,	false },
			{ "parry_chance",			"PARRY",		"Parry Chance",								"Improves your chance to parry an attack by %u%%.",						10.0f,	true,	false },
			{ "block_value",			"BLOCKVALUE",	"Block Value",								"Increases damage absorbed by blocks by %u.",							1.0f,	false,	false },
			{ "attack_haste",			"HASTE",		"Attack Haste",								"Increases attack speed by %u%%.",										10.0f,	true,	false },
			{ "spell_haste",			"SPELLHASTE",	"Spell Haste",								"Increases cast speed by %u%%.",										10.0f,	true,	false },
			{ "spell_damage",			"SPELLDMG",		"Spell Damage",								NULL,																	1.0f,	false,	false },
			{ "healing",				"HEALING",		"Healing",									NULL,																	1.0f,	false,	false },
			{ "health_per_5",			"HP5",			"Health Per 5 Seconds",						NULL,																	1.0f,	false,	false },
			{ "mana_per_5",				"MP5",			"Mana Per 5 Seconds",						NULL,																	1.0f,	false,	false },
			{ "attack_power",			"AP",			"Attack Power",								NULL,																	2.0f,	false,	false },
			{ "stealth",				"STEALTH",		"Stealth",									"Decreases likelihood of detection when stealthed.",					10.0f,	false,	false },
			{ "resilience",				"RES",			"Resilience",								"Reduces your chance of being hit by a critical strike by %u%%.",		10.0f,	true,	false },
			{ "hit_chance",				"HIT",			"Hit Chance",								"Improves your chance to hit by %u%%.",									10.0f,	true,	false },
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_IDS);

		inline constexpr const Info*
		GetInfo(
			Id				aId)
		{
			assert((uint32_t)aId < (uint32_t)NUM_IDS);
			return &INFO[aId];
		}

		inline constexpr Id
		StringToId(
			const char*		aString)
		{
			std::string_view s(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if(s == t->m_name)
					return (Id)i;
			}
			return INVALID_ID;
		}

		struct Collection
		{
			Collection()
			{
			}

			void
			Reset()
			{
				memset(m_stats, 0, sizeof(m_stats));
			}

			bool
			IsEmpty() const
			{
				for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
				{
					if(m_stats[i] != 0.0f)
						return false;
				}
				return true;
			}

			void
			Add(
				const Collection&	aOther)
			{
				for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
					m_stats[i] += aOther.m_stats[i];
			}

			void
			FromSource(
				const SourceNode*	aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aNode)
				{
					Id id = StringToId(aNode->m_name.c_str());
					TP_VERIFY(id != INVALID_ID, aNode->m_debugInfo, "'%s' is not a valid stat.", aNode->m_name.c_str());
					m_stats[id] = aNode->GetFloat();
				});
			}

			void
			ToStream(
				IWriter*			aStream) const 
			{
				for(uint32_t i = 0; i < (uint32_t)NUM_IDS; i++)
					aStream->WriteFloat(m_stats[i]);
			}

			bool
			FromStream(
				IReader*			aStream) 
			{
				for (uint32_t i = 0; i < (uint32_t)NUM_IDS; i++)
				{
					if(!aStream->ReadFloat(m_stats[i]))
						return false;
				}
				return true;
			}

			void
			DebugPrint() const
			{
				for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
				{
					float value = m_stats[i];
					
					if(value > 0)
						printf(" %s=%f", GetInfo((Id)i)->m_shortName, value);
				}
			}
			
			bool
			Compare(
				const Collection&	aOther) const
			{
				for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
				{
					if(m_stats[i] != aOther.m_stats[i])
						return false;
				}
				return true;
			}

			void
			RemoveLowStats(
				uint32_t			aNumberToKeep)
			{
				// Only keep the X highest stats, reset the rest to 0
				struct Entry
				{
					uint32_t	m_index = 0;
					float		m_value = 0;
				};

				std::vector<Entry> entries;

				for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
				{
					if(m_stats[i] > 0.0f)
						entries.push_back({ i, m_stats[i] });
				}

				if(entries.size() > (size_t)aNumberToKeep)
				{
					std::sort(entries.begin(), entries.end(), [](
						const Entry& aLHS, 
						const Entry& aRHS)
					{
						return aLHS.m_value > aRHS.m_value;
					});

					for(size_t i = (size_t)aNumberToKeep; i < entries.size(); i++)
						m_stats[entries[i].m_index] = 0.0f;
				}
			}

			float
			GetTotalBudgetCost() const
			{
				float sum = 0;
				for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
				{
					if(m_stats[i] != 0.0f)
					{
						const Info* statInfo = GetInfo((Id)i);
						sum += statInfo->m_budgetCost * m_stats[i];
					}
				}
				return sum;
			}

			uint32_t
			GetHash() const
			{
				Hash::CheckSum hash;
				hash.AddPOD(m_stats);
				return hash.m_hash;
			}

			// Public data
			float		m_stats[NUM_IDS] = { 0 };
		};

		struct Conversion
		{
			Conversion()
			{

			}

			Conversion(
				const SourceNode*	aSource)
			{
				aSource->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (aChild->m_name == "from")
						m_fromStatId = StringToId(aChild->GetIdentifier());
					else if (aChild->m_name == "to")
						m_toStatId = StringToId(aChild->GetIdentifier());
					else if (aChild->m_name == "factor")
						m_factor = aChild->GetFloat();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*			aStream) const 
			{
				aStream->WritePOD(m_fromStatId);
				aStream->WritePOD(m_toStatId);
				aStream->WriteFloat(m_factor);
			}

			bool
			FromStream(
				IReader*			aStream) 
			{
				if (!aStream->ReadPOD(m_fromStatId))
					return false;
				if (!aStream->ReadPOD(m_toStatId))
					return false;
				if (!aStream->ReadFloat(m_factor))
					return false;
				return true;
			}

			// Public data
			Id			m_fromStatId = INVALID_ID;
			Id			m_toStatId = INVALID_ID;
			float		m_factor = 0;
		};

	};

}