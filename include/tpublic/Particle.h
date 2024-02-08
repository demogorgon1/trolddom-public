#pragma once

namespace tpublic
{

	namespace Particle
	{

		enum Flag : uint8_t
		{
			FLAG_ATTACHED		= 0x01,
			FLAG_NO_ROTATION	= 0x02
		};

		// IMPORTANT: Must match Flag enum
		static constexpr const char* FLAG_NAMES[] =
		{
			"attached",
			"no_rotation",
			NULL
		};

		inline constexpr uint8_t
		StringToFlag(
			const char* aString)
		{
			std::string_view s(aString);
			for (uint8_t i = 0; i < 8 && FLAG_NAMES[i] != NULL; i++)
			{
				if(s == FLAG_NAMES[i])
					return 1 << i;
			}
			return 0;
		}

	}

}