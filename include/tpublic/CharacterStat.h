#pragma once

#include "ComponentBase.h"
#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	namespace CharacterStat
	{

		// IMPORTANT: Never remove or reorder anything in this enum, only add new things
		enum Id : uint32_t
		{
			INVALID_ID,

			ID_CASH_LOOTED,
			ID_TOTAL_KILLS,
			ID_QUESTS_COMPLETED,
			ID_TIME_PLAYED,
			ID_MAX_UNCOMMON_EQUIP,
			ID_MAX_RARE_EQUIP,
			ID_MAX_EPIC_EQUIP,
			ID_TOTAL_PVP_KILLS,
			ID_BANDAGES_CRAFTED,
			ID_MEALS_PREPARED,
			ID_FISH_CAUGHT,
			ID_AREAS_DISCOVERED,

			NUM_IDS
		};

		struct Info
		{
			const char*								m_name;
			const char*								m_displayFormat;
			ComponentBase::PendingPersistenceUpdate	m_persistenceUpdate;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL, ComponentBase::PENDING_PERSISTENCE_UPDATE_NONE },

																						// The priority of saving stats after an update
			{ "cash_looted",		"Cash looted: {%zu}",								ComponentBase::PENDING_PERSISTENCE_UPDATE_NONE },
			{ "total_kills",		"Total kills: %zu",									ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY },
			{ "quests_completed",	"Quests completed: %zu",							ComponentBase::PENDING_PERSISTENCE_UPDATE_MEDIUM_PRIORITY },
			{ "time_played",		"Time played: {@%zu}",								ComponentBase::PENDING_PERSISTENCE_UPDATE_NONE },
			{ "max_uncommon_equip", "Maximum number of uncommon items equipped: %zu",	ComponentBase::PENDING_PERSISTENCE_UPDATE_NONE },
			{ "max_rare_equip",		"Maximum number of rare items equipped: %zu",		ComponentBase::PENDING_PERSISTENCE_UPDATE_NONE },
			{ "max_epic_equip",		"Maximum number of epic items equipped: %zu",		ComponentBase::PENDING_PERSISTENCE_UPDATE_NONE },
			{ "total_pvp_kills",	"Total player kills: %zu",							ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY },
			{ "bandages_crafted",	"Total bandages crafted: %zu",						ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY },
			{ "meals_prepared",		"Total meals prepared: %zu",						ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY },
			{ "fish_caught",		"Total fish caught: %zu",							ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY },
			{ "areas_discovered",	"Areas discovered: %zu",							ComponentBase::PENDING_PERSISTENCE_UPDATE_NONE },
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
			const char* aString)
		{
			std::string_view s(aString);
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if (s == t->m_name)
					return (Id)i;
			}
			return INVALID_ID;
		}

		inline constexpr bool
		ValidateId(
			Id				aId)
		{
			return (uint32_t)aId >= 1 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}

		class Collection
		{
		public:
			void		
			ToStream(
				IWriter*		aWriter) const
			{
				aWriter->WriteUInt<uint32_t>((uint32_t)NUM_IDS);
				for(uint32_t i = 0; i < (uint32_t)NUM_IDS; i++)
					aWriter->WriteUInt(m_value[i]);
			}

			bool
			FromStream(
				IReader*		aReader)
			{
				uint32_t count;
				if(!aReader->ReadUInt(count))
					return false;

				for(uint32_t i = 0; i < count; i++)
				{
					if(!aReader->ReadUInt(m_value[i]))
						return false;
				}

				if(count < (uint32_t)NUM_IDS)
				{
					for(uint32_t i = count; i < (uint32_t)NUM_IDS; i++)
						m_value[i] = 0;
				}

				return true;
			}			

			void
			Reset()
			{
				memset(m_value, 0, sizeof(m_value));
			}

			// Public data
			uint64_t	m_value[NUM_IDS] = { 0 };
		};

	}

}