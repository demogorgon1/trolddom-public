#pragma once

namespace tpublic
{

	namespace AuraEffect
	{

		enum Id : uint32_t
		{
			INVALID_ID,

			ID_STUN,
			ID_IMMORTALITY,
			ID_DAMAGE_INPUT_MODIFIER,
			ID_REPEAT,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* EFFECT_NAMES[] =
		{
			NULL,
			"stun",
			"immortality",
			"damage_input_modifier",
			"repeat"
		};

		static_assert(sizeof(EFFECT_NAMES) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			std::string_view s(aString);
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				assert(EFFECT_NAMES[i] != NULL);
				if (s == EFFECT_NAMES[i])
					return (Id)i;
			}

			return INVALID_ID;
		}

	}

}