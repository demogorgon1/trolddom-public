#pragma once

namespace tpublic
{

	namespace Stealth
	{

		uint32_t	GetOneSecondDetectionChance(
						int32_t		aDistanceSquared,
						uint8_t		aStealthLevel,
						uint32_t	aLevel,
						uint32_t	aTargetLevel);

	}

}