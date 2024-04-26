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
			return (int32_t)((float)aBaseCastTime * (1.0f - aHaste / 100.0f));
		}

	}

}