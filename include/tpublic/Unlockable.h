#pragma once

#include "SourceNode.h"

namespace tpublic
{

	namespace Unlockable
	{

		enum Flag : uint32_t
		{
			FLAG_CHARACTER_CUSTOMIZATION,

			NUM_FLAGS,
			INVALID_FLAG
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ "character_customization", "Character Customization" }
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_FLAGS);

		inline constexpr const Info*
		GetInfo(
			Flag				aFlag)
		{
			assert((uint32_t)aFlag < (uint32_t)NUM_FLAGS);
			return &INFO[aFlag];
		}

		inline constexpr Flag
		StringToFlag(
			const char*			aString)
		{
			std::string_view s(aString);
			for(uint32_t i = 0; i < (uint32_t)NUM_FLAGS; i++)
			{
				const Info* t = &INFO[i];
				if(s == t->m_name)
					return (Flag)i;
			}
			return INVALID_FLAG;
		}

		inline constexpr bool
		ValidateFlag(
			Flag				aFlag)
		{
			return (uint32_t)aFlag < (uint32_t)NUM_FLAGS;
		}

		inline const uint32_t
		SourceToFlags(
			const SourceNode*	aSource)
		{
			uint32_t flags = 0;
			
			aSource->ForEachChild([&](const SourceNode* aChild)
			{
				Flag flag = StringToFlag(aChild->GetIdentifier());
				if (ValidateFlag(flag))
					flags |= (1 << (uint32_t)flag);
			});

			return flags;
		}
	}

}