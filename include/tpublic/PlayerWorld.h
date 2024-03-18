#pragma once

#include "Hash.h"
#include "IReader.h"
#include "IWriter.h"
#include "Vec2.h"

namespace tpublic
{

	namespace PlayerWorld
	{

		enum Size : uint8_t
		{
			INVALID_SIZE,

			SIZE_SMALL,

			NUM_SIZES
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayName;
			Vec2		m_size;
		};

		// IMPORTANT: Must match Type enum
		static constexpr const Info INFO[] =
		{
			{ NULL,					NULL,			Vec2() },

			{ "small",				"Small",		Vec2(64, 64) },
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_SIZES);

		inline constexpr const Info*
		GetInfo(
			Size				aSize)
		{
			assert((uint32_t)aSize < (uint32_t)NUM_SIZES);
			return &INFO[aSize];
		}

		inline constexpr Size
		StringToSize(
			const char*			aString)
		{
			std::string_view s(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_SIZES; i++)
			{
				const Info* t = &INFO[i];
				if(s == t->m_name)
					return (Size)i;
			}
			return INVALID_SIZE;
		}

		inline constexpr bool
		ValidateSize(
			Size				aSize)
		{
			return (uint32_t)aSize >= 1 && (uint32_t)aSize < (uint32_t)NUM_SIZES;
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

			bool
			IsSet() const
			{
				return m_characterId != 0 && m_playerWorldId != 0;
			}

			// Public data
			uint32_t		m_characterId = 0;
			uint32_t		m_playerWorldId = 0;
		};

	}

}