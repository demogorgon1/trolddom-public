#pragma once

namespace tpublic
{
	
	namespace CombatEvent
	{

		enum Id : uint8_t
		{
			// Order is prioritized: if multiple combat events are associated with same ability, highest value takes precedence
			ID_MISS,
			ID_DODGE,
			ID_PARRY,
			ID_BLOCK,
			ID_HIT,
			ID_CRITICAL,

			NUM_IDS,

			INVALID_ID
		};

		constexpr bool 
		IsHit(
			Id			aId)
		{
			switch(aId)
			{
			case ID_HIT:
			case ID_CRITICAL:
			case ID_BLOCK:
				return true;

			default:
				return false;
			}
		}

		constexpr Id
		StringToId(
			const char*	aString)
		{
			std::string_view t(aString);
			if (t == "hit")
				return ID_HIT;
			if (t == "miss")
				return ID_MISS;
			if (t == "dodge")
				return ID_DODGE;
			if (t == "parry")
				return ID_PARRY;
			if (t == "block")
				return ID_BLOCK;
			if (t == "critical")
				return ID_CRITICAL;
			return INVALID_ID;
		}

		constexpr const char*
		IdToString(
			Id			aId)
		{
			switch(aId)
			{
			case ID_MISS:		return "miss";
			case ID_DODGE:		return "dodge";
			case ID_PARRY:		return "parry";
			case ID_BLOCK:		return "block";
			case ID_HIT:		return "hit";
			case ID_CRITICAL:	return "critical";
			}
			return "";
		}

	}

}