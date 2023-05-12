#pragma once

namespace tpublic
{
	
	namespace CombatEvent
	{

		enum Id : uint8_t
		{
			ID_HIT,
			ID_MISS,
			ID_DODGE,
			ID_PARRY,
			ID_BLOCK,
			ID_CRITICAL,

			NUM_IDS
		};

		constexpr bool 
		IsHit(
			Id		aId)
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

	}

}