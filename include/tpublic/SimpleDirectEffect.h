#pragma once

#include "DataType.h"

namespace tpublic
{

	namespace SimpleDirectEffect
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_OPEN,
			ID_PRAY,
			ID_MAKE_OFFERING,
			ID_KILL,
			ID_PUSH,
			ID_DESECRATE,
			ID_START_QUEST,
			ID_EDIT_PLAYER_WORLDS,
			ID_PASS,
			ID_RESURRECT,
			ID_ACTIVATE_TRIGGER,
			ID_SET_OWN_STATE,
			ID_RECALL,
			ID_LEARN_RIDING,
			ID_CAPTURE,
			ID_THREAT_TABLE_SHUFFLE,
			ID_THREAT_TABLE_EQUALIZE,
			ID_ZONE_TELEPORT,
			ID_ENTITY_COMPASS,
			ID_SET_MAP_TRIGGER,

			NUM_IDS
		};

		struct Info
		{
			const char*		m_name;
			DataType::Id	m_paramType;
			bool			m_isEntityState;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL,						DataType::INVALID_ID,		false },

			{ "open",					DataType::INVALID_ID,		false },
			{ "pray",					DataType::ID_PANTHEON,		false },
			{ "make_offering",			DataType::INVALID_ID,		false },
			{ "kill",					DataType::INVALID_ID,		false },
			{ "push",					DataType::INVALID_ID,		false },
			{ "desecrate",				DataType::INVALID_ID,		false },
			{ "start_quest",			DataType::ID_QUEST,			false },
			{ "edit_player_worlds",		DataType::INVALID_ID,		false },
			{ "pass",					DataType::INVALID_ID,		false },
			{ "resurrect",				DataType::INVALID_ID,		false },
			{ "activate_trigger",		DataType::INVALID_ID,		false },
			{ "set_own_state",			DataType::INVALID_ID,		true },
			{ "recall",					DataType::INVALID_ID,		false },
			{ "learn_riding",			DataType::INVALID_ID,		false },
			{ "capture",				DataType::INVALID_ID,		false },
			{ "threat_table_shuffle",	DataType::INVALID_ID,		false },
			{ "threat_table_equalize",	DataType::INVALID_ID,		false },
			{ "zone_teleport",			DataType::ID_ZONE,			false },
			{ "entity_compass",			DataType::ID_ENTITY,		false },
			{ "set_map_trigger",		DataType::ID_MAP_TRIGGER,	false },
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

		inline constexpr bool
		ValidateId(
			Id				aId)
		{
			return (uint32_t)aId >= 1 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}

	}

}