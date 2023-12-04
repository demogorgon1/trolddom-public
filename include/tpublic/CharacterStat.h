#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	namespace CharacterStat
	{

		// IMPORTANT: Never remove anything from this enum, only add new things
		enum Id : uint32_t
		{
			INVALID_ID,

			ID_CASH_LOOTED,

			NUM_IDS
		};

		struct Info
		{
			const char* m_name;
			const char* m_displayFormat;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL, NULL },

			{ "cash_looted", "{%zu} looted" }
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_IDS);

		inline constexpr const Info*
		GetInfo(
			Id				aId)
		{
			assert((uint32_t)aId < (uint32_t)NUM_IDS);
			return &INFO[aId];
		}

		inline constexpr Id
		StringToId(
			const char* aString)
		{
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if (strcmp(t->m_name, aString) == 0)
					return (Id)i;
			}
			return INVALID_ID;
		}

		inline constexpr bool
		ValidateId(
			Id				aId)
		{
			return (uint32_t)aId >= 1 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}

		class Collection
		{
		public:
			void		
			ToStream(
				IWriter*		aWriter) const
			{
				aWriter->WriteUInt<uint32_t>((uint32_t)NUM_IDS);
				for(uint32_t i = 0; i < (uint32_t)NUM_IDS; i++)
					aWriter->WriteUInt(m_value[i]);
			}

			bool
			FromStream(
				IReader*		aReader)
			{
				uint32_t count;
				if(!aReader->ReadUInt(count))
					return false;

				for(uint32_t i = 0; i < count; i++)
				{
					if(!aReader->ReadUInt(m_value[i]))
						return false;
				}

				if(count < (uint32_t)NUM_IDS)
				{
					for(uint32_t i = count; i < (uint32_t)NUM_IDS; i++)
						m_value[i] = 0;
				}

				return true;
			}			

			// Public data
			uint64_t	m_value[NUM_IDS];
		};

	}

}