#pragma once

namespace tpublic
{

	namespace RealmModifier
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_HARDCORE_MODE,
			ID_XP_MULTIPLIER,

			NUM_IDS
		};

		struct Info
		{
			enum Type : uint8_t
			{
				INVALID_TYPE,

				TYPE_FLAG,
				TYPE_MULTIPLIER
			};

			// Helpers
			size_t 
			GetValidOptionIndex(
				const char*	aString) const
			{
				std::string_view t(aString);
				for(size_t i = 0; i < m_validOptions.size(); i++)
				{
					if(t == m_validOptions[i])
						return i;
				}
				return SIZE_MAX;
			}

			// Public data
			const char*					m_string;
			const char*					m_displayName;
			Type						m_type;
			std::vector<const char*>	m_validOptions;
			const char*					m_description;
			const char*					m_default;
		};

		// IMPORTANT: Must match Id enum
		static const Info INFO[] =
		{
			{ NULL, NULL, Info::INVALID_TYPE, {}, NULL },

			{ "hardcore_mode", "Hardcore Mode", Info::TYPE_FLAG, { "true", "false" }, "Permanent death.", "false" },
			{ "xp_multiplier", "XP Gain", Info::TYPE_MULTIPLIER, { "50%", "100%", "150%", "200%", "300%" }, "XP gained from kills and quests.", "100%" }
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_IDS);

		inline constexpr const Info*
		GetInfo(
			Id				aId)
		{
			assert((uint32_t)aId < (uint32_t)NUM_IDS);
			return &INFO[aId];
		}

		inline constexpr bool
		ValidateId(
			Id				aId)
		{
			return (uint32_t)aId >= 1 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}

		inline constexpr Id
		StringToId(
			const char*		aString)
		{
			std::string_view t(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				if(t == INFO[i].m_string)
					return (Id)i;
			}
			return INVALID_ID;
		}


	}

}