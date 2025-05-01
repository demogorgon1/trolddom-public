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
			ID_COMBAT_EVENT_TRIGGER,
			ID_HEAL_INPUT_MODIFIER,
			ID_MOVE_SPEED_MODIFIER,
			ID_DAMAGE_OUTPUT_MODIFIER,
			ID_TAUNT,
			ID_THREAT_MODIFIER,
			ID_HEAL_OUTPUT_MODIFIER,
			ID_IMMOBILIZE,
			ID_ABSORB,
			ID_CAST_MODIFIER,
			ID_ABILITY_ON_FADE,
			ID_DAMAGE_INPUT_EFFECT,
			ID_BLIND,	
			ID_STEALTH,
			ID_RESOURCE_COST_MODIFIER,

			NUM_IDS
		};

		// IMPORTANT: Must match Id enum
		static constexpr const char* EFFECT_NAMES[] =
		{
			NULL,
			"stun",
			"immortality",
			"damage_input_modifier",
			"repeat",
			"combat_event_trigger",
			"heal_input_modifier",
			"move_speed_modifier",
			"damage_output_modifier",
			"taunt",
			"threat_modifier",
			"heal_output_modifier",
			"immobilize",
			"absorb",
			"cast_modifier",
			"ability_on_fade",
			"damage_input_effect",
			"blind",
			"stealth",
			"resource_cost_modifier"
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

		inline constexpr const char*
		IdToString(
			Id			aId)
		{
			assert((uint32_t)aId > INVALID_ID && (uint32_t)aId < NUM_IDS);
			return EFFECT_NAMES[aId];
		}

	}

}