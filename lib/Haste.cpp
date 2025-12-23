#include "Pcheader.h"

namespace tpublic
{


	namespace Haste
	{

		int32_t	
		CalculateCastTime(
			int32_t						aBaseCastTime,
			float						aHaste)
		{
			//return (int32_t)((float)aBaseCastTime * (1.0f - aHaste / 100.0f));

			float x = aHaste / 100.0f;
			float y = 0.5892858f * x + 1.2857139f * x * x - 1.053f * x * x * x; // Fitted 3rd degree polynomial for smooth diminishing return after ~50%

			return (int32_t)((float)aBaseCastTime * (1.0f - y));
		}

	}

}