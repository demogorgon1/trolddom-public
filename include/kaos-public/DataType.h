#pragma once

namespace kaos_public
{

	namespace DataType
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_ABILITY,
			ID_CLASS,
			ID_ENTITY,
			ID_MAP,
			ID_TALENT,
			ID_TALENT_TREE,

			NUM_IDS
		};

		// IMPORTANT: must match Id enum
		static constexpr const char* STRINGS[] =
		{
			NULL,
			"ability",
			"class",
			"entity",
			"map",
			"talent",
			"talent_tree"
		};

		static_assert(sizeof(STRINGS) / sizeof(const char*) == (size_t)NUM_IDS);

		inline constexpr Id 
		StringToId(
			const char*		aString)
		{
			for(uint8_t i = 1; i < (uint8_t)NUM_IDS; i++)
			{
				if(strcmp(STRINGS[i], aString) == 0)
					return (Id)i;
			}

			return INVALID_ID;
		}

		inline constexpr const char* 
		IdToString(
			Id				aId)
		{
			assert((uint8_t)aId < (uint8_t)NUM_IDS);
			const char* p = STRINGS[aId];
			assert(p != NULL);
			return p;
		}

	}

}