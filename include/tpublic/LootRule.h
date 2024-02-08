#pragma once

namespace tpublic
{
	
	namespace LootRule
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_FREE_FOR_ALL,
			ID_GROUP,
			ID_MASTER,

			NUM_IDS,
		};

		struct Info
		{
			const char*		m_displayName;
			const char*		m_string;
			const char*		m_description;
		};

		// IMPORTANT: Must match Id enum
		static constexpr Info INFO[] = 
		{
			{ NULL, NULL, NULL }, 
			{ "Free For All",	"free_for_all",	"Anyone can loot anything." },
			{ "Group",			"group",		"Items at the rarity threshold or above are rolled for." },
			{ "Master",			"master",		"Items at the rarity threshold or above are assigned by leader." }
		};

		static_assert(sizeof(INFO) == sizeof(Info) * (size_t)NUM_IDS);

		static constexpr const char*
		IdToDisplayName(
			Id			aId)
		{
			assert((uint8_t)aId >= 1 && (uint8_t)aId < NUM_IDS);
			return INFO[aId].m_displayName;
		}

		static constexpr const char*
		IdToDescription(
			Id			aId)
		{
			assert((uint8_t)aId >= 1 && (uint8_t)aId < NUM_IDS);
			return INFO[aId].m_description;
		}

		static constexpr Id
		StringToId(
			const char*	aString)
		{	
			std::string_view s(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* info = &INFO[i];
				if(s == info->m_string)
					return (Id)i;
			}
			return INVALID_ID;
		}

		static constexpr bool
		ValidateId(
			Id			aId)
		{
			return (uint8_t)aId >= 1 && (uint8_t)aId < NUM_IDS;
		}
	}

}