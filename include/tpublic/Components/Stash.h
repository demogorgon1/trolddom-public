#pragma once

#include "../ComponentBase.h"
#include "../ItemList.h"

namespace tpublic
{

	namespace Components
	{

		struct Stash
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_STASH;
			static const uint8_t FLAGS = FLAG_REPLICATE_ONLY_ON_REQUEST | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			static const uint32_t SOFT_SIZE_LIMIT = 16;		

			enum Field : uint32_t
			{
				FIELD_ITEMS,
				FIELD_EXTRA_SLOTS
			};

			static void
			CreateSchema(
				ComponentSchema*					aSchema)
			{
				aSchema->DefineCustomObjectNoSource<ItemList>(FIELD_ITEMS, offsetof(Stash, m_items));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_EXTRA_SLOTS, NULL, offsetof(Stash, m_extraSlots));

				aSchema->OnRead<Stash>([](
					Stash*							aStash,
					ComponentSchema::ReadType		aReadType,
					const Manifest*					aManifest)
				{
					if(aReadType == ComponentSchema::READ_TYPE_STORAGE)
						aStash->m_items.OnLoadedFromPersistence(aManifest, aStash->GetSoftSizeLimit());
				});
			}

			void
			Reset()
			{
				m_items.Reset();
				m_extraSlots = 0;
			}

			uint32_t 
			GetSoftSizeLimit() const
			{
				return m_extraSlots + SOFT_SIZE_LIMIT;
			}

			// Public data
			ItemList		m_items;
			uint32_t		m_extraSlots = 0;
		};
	}

}