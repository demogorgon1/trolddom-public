#pragma once

#include "Parser.h"

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

			NUM_IDS
		};
		
		struct Info
		{
			const char*		m_name;
			const char*		m_shortName;
			const char*		m_longName;
			const char*		m_description;
			uint32_t		m_budgetCost;
			bool			m_percentage;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{			
			{ NULL, NULL, NULL, NULL, 0, false },

			{ "dexterity",				"DEX",			"Dexterity",								NULL,																	1,	false },
			{ "strength",				"STR",			"Strength",									NULL,																	1,	false },
			{ "wisdom",					"WIS",			"Wisdom",									NULL,																	1,	false },
			{ "constitution",			"CON",			"Constitution",								NULL,																	1,	false },
			{ "spirit",					"SPI",			"Spirit",									NULL,																	1,	false },
			{ "armor",					"ARM",			"Armor",									NULL,																	1,	false },
			{ "phys_crit_chance",		"CRIT",			"Critical Strike Chance (Physical)",		"Improves your chance to get a critical strike by %u%%.",				10,	true },
			{ "spell_crit_chance",		"SPELLCRIT",	"Critical Strike Chance (Spells)",			"Improves your chance to get a critical strike with spells by %u%%.",	10,	true },
			{ "dodge_chance",			"DODGE",		"Dodge Chance",								"Improves your chance to dodge an attack by %u%%.",						10,	true },
			{ "block_chance",			"BLOCK",		"Block Chance",								"Improves your chance to block an attack by %u%%.",						10,	true },
			{ "parry_chance",			"PARRY",		"Parry Chance",								"Improves your chance to parry an attack by %u%%.",						10,	true }
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
					if(m_stats[i] != 0)
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
					m_stats[id] = aNode->GetUInt32();
				});
			}

			void
			ToStream(
				IWriter*			aStream) const 
			{
				for(uint32_t i = 0; i < (uint32_t)NUM_IDS; i++)
					aStream->WriteUInt(m_stats[i]);
			}

			bool
			FromStream(
				IReader*			aStream) 
			{
				for (uint32_t i = 0; i < (uint32_t)NUM_IDS; i++)
				{
					if(!aStream->ReadUInt(m_stats[i]))
						return false;
				}
				return true;
			}

			void
			DebugPrint() const
			{
				for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
				{
					uint32_t value = m_stats[i];
					
					if(value > 0)
						printf(" %s=%u", GetInfo((Id)i)->m_shortName, value);
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
					uint32_t	m_value = 0;
				};

				std::vector<Entry> entries;

				for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
				{
					if(m_stats[i] > 0)
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
						m_stats[entries[i].m_index] = 0;
				}
			}

			uint32_t
			GetTotalBudgetCost() const
			{
				uint32_t sum = 0;
				for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
				{
					if(m_stats[i] != 0)
					{
						const Info* statInfo = GetInfo((Id)i);
						sum += statInfo->m_budgetCost * m_stats[i];
					}
				}
				return sum;
			}

			// Public data
			uint32_t		m_stats[NUM_IDS] = { 0 };
		};

	};

}