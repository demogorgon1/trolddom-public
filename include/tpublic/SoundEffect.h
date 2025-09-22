#pragma once

#include "Data/Sound.h"

#include "SourceNode.h"

namespace tpublic
{

	namespace SoundEffect
	{

		enum Id : uint8_t
		{
			INVALID_ID,

			ID_MOVE,
			ID_HIT,
			ID_MISS,
			ID_BLOCK,
			ID_PARRY,
			ID_DODGE,
			ID_USE,
			ID_USE_LOCAL,
			ID_EQUIP,
			ID_INVENTORY,
			ID_VENDOR,
			ID_TRASH,

			NUM_IDS
		};
		
		struct Info
		{
			const char*		m_name;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{			
			{ NULL },

			{ "move" },
			{ "hit" },
			{ "miss" },
			{ "block" },
			{ "parry" },
			{ "dodge" },
			{ "use" },
			{ "use_local" },
			{ "equip" },
			{ "inventory" },
			{ "vendor" },
			{ "trash" },
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
			const char*		aString)
		{
			std::string_view s(aString);
			for(uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if(s == t->m_name)
					return (Id)i;
			}
			return INVALID_ID;
		}

		struct Collection
		{
			Collection()
			{
			}

			void
			FromSource(
				const SourceNode*	aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aNode)
				{
					Id id = StringToId(aNode->m_name.c_str());
					TP_VERIFY(id != INVALID_ID, aNode->m_debugInfo, "'%s' is not a valid sound effect.", aNode->m_name.c_str());
					aNode->GetIdArray(DataType::ID_SOUND, m_sounds[id]);
				});
			}

			void
			ToStream(
				IWriter*			aStream) const 
			{
				for(uint32_t i = 0; i < (uint32_t)NUM_IDS; i++)
					aStream->WriteUInts(m_sounds[i]);
			}

			bool
			FromStream(
				IReader*			aStream) 
			{
				for (uint32_t i = 0; i < (uint32_t)NUM_IDS; i++)
				{
					if(!aStream->ReadUInts(m_sounds[i]))
						return false;
				}
				return true;
			}

			uint32_t
			GetRandomSound(
				std::mt19937&		aRandom,
				Id					aId) const
			{
				const std::vector<uint32_t>& t = m_sounds[aId];
				if(t.empty())
					return 0;

				if(t.size() == 1)
					return t[0];
				
				tpublic::UniformDistribution<size_t> distribution(0, t.size() - 1);
				return t[distribution(aRandom)];
			}

			// Public data
			std::vector<uint32_t>		m_sounds[NUM_IDS];
		};

	};

}