#pragma once

namespace kpublic
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
			ID_CRITICAL_STRIKE,

			NUM_IDS
		};

	}

}