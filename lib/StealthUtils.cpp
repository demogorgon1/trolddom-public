#include "Pcheader.h"

#include <tpublic/Helpers.h>

namespace tpublic
{

	namespace StealthUtils
	{

		uint32_t
		GetOneSecondDetectionChance(
			int32_t		aDistanceSquared,
			uint8_t		aStealthLevel,
			uint32_t	aLevel,
			uint32_t	aTargetLevel)
		{	
			float d = sqrtf((float)aDistanceSquared);
			float s = (float)aStealthLevel;
			float p = 0.125f / (d * s);

			if(aTargetLevel < aLevel)
				p *= 1.5f;

			if (aTargetLevel > aLevel)
				p *= 0.75f;

			return (uint32_t)(tpublic::Helpers::Clamp<float>(p, 0.0f, 1.0f) * (uint32_t)UINT32_MAX);
		}

	}

}