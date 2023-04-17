#pragma once

#include "Parser.h"

namespace kaos_public
{

	namespace Stat
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_DEXTERITY,
			ID_STRENGTH,
			ID_WISDOM,
			ID_CRITICAL_STRIKE_CHANCE,
			ID_DODGE_CHANCE,
			ID_PARRY_CHANCE,

			NUM_IDS
		};
		
		struct Info
		{
			const char*		m_name;
			const char*		m_shortName;
			const char*		m_longName;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{			
			{ NULL, NULL, NULL },

			{ "dexterity",				"DEX",		"Dexterity" },
			{ "strength",				"STR",		"Strength" },
			{ "wisdom",					"WIS",		"Wisdom" },
			{ "critical_strike_chance",	"CRI",		"Critical Strike Chance" },
			{ "dodge_chance",			"DOD",		"Dodge Chance" },
			{ "parry_chance",			"PAR",		"Parry Chance" }
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
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if(strcmp(t->m_name, aString) == 0)
					return (Id)i;
			}
			return INVALID_ID;
		}

		struct Collection
		{
			Collection()
			{
				memset(m_stats, 0, sizeof(m_stats));
			}

			void
			Reset()
			{
				memset(m_stats, 0, sizeof(m_stats));
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
				const Parser::Node*	aSource)
			{
				aSource->ForEachChild([&](
					const Parser::Node* aNode)
				{
					Id id = StringToId(aNode->m_name.c_str());
					KP_VERIFY(id != INVALID_ID, aNode->m_debugInfo, "'%s' is not a valid stat.", aNode->m_name.c_str());
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

			// Public data
			uint32_t		m_stats[NUM_IDS];
		};

	};

}