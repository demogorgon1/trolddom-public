#pragma once

#include "Hash.h"
#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	namespace PlayerWorld
	{

		enum Type : uint8_t
		{
			INVALID_TYPE,

			TYPE_SMALL_ISLAND,

			NUM_TYPES
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;
		};

		// IMPORTANT: Must match Type enum
		static constexpr const Info INFO[] =
		{
			{ NULL,					NULL },

			{ "small_island",		"Small Island" },
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_TYPES);

		inline constexpr const Info*
		GetInfo(
			Type				aType)
		{
			assert((uint32_t)aType < (uint32_t)NUM_TYPES);
			return &INFO[aType];
		}

		inline constexpr Type
		StringToType(
			const char*			aString)
		{
			std::string_view s(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_TYPES; i++)
			{
				const Info* t = &INFO[i];
				if(s == t->m_name)
					return (Type)i;
			}
			return INVALID_TYPE;
		}

		inline constexpr bool
		ValidateType(
			Type				aType)
		{
			return (uint32_t)aType >= 1 && (uint32_t)aType < (uint32_t)NUM_TYPES;
		}

		class Key
		{
		public:
			struct Hasher
			{
				uint32_t
				operator()(
					const Key&	aKey) const
				{
					return (uint32_t)Hash::Splitmix_2_32(aKey.m_characterId, aKey.m_playerWorldId);
				}
			};

			bool
			operator==(
				const Key&		aOther) const
			{
				return m_characterId == aOther.m_characterId && m_playerWorldId == aOther.m_playerWorldId;
			}

			void
			ToStream(
				IWriter*		aWriter) const
			{
				aWriter->WriteUInt(m_characterId);
				aWriter->WriteUInt(m_playerWorldId);
			}

			bool
			FromStream(
				IReader*		aReader)
			{
				if (!aReader->ReadUInt(m_characterId))
					return false;
				if (!aReader->ReadUInt(m_playerWorldId))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_characterId = 0;
			uint32_t		m_playerWorldId = 0;
		};

	}

}