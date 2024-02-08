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

			NUM_IDS
		};

		struct Info
		{
			const char*		m_name;
			DataType::Id	m_paramType;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL,					DataType::INVALID_ID },

			{ "open",				DataType::INVALID_ID },
			{ "pray",				DataType::ID_PANTHEON },
			{ "make_offering",		DataType::INVALID_ID },
			{ "kill",				DataType::INVALID_ID },
			{ "push",				DataType::INVALID_ID },
			{ "desecrate",			DataType::INVALID_ID },
			{ "start_quest",		DataType::ID_QUEST }
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