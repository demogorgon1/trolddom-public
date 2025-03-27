#include "../Pcheader.h"

#include <tpublic/Components/Inventory.h>
#include <tpublic/Components/PlayerPrivate.h>

#include <tpublic/DirectEffects/ConsumeAndProduce.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace DirectEffects
	{

		void
		ConsumeAndProduce::FromSource(
			const SourceNode*				aSource) 
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if (!FromSourceBase(aChild))
				{
					if (aChild->m_name == "consume_source_item")
						m_consumeSourceItemId = aChild->GetId(DataType::ID_ITEM);
					else if (aChild->m_name == "consume_source_item_count")
						m_consumeSourceItemCount = aChild->GetUInt32();
					else if (aChild->m_name == "produce_source_item")
						m_produceSourceItemId = aChild->GetId(DataType::ID_ITEM);
					else if (aChild->m_name == "produce_source_item_count")
						m_produceSourceItemCount = aChild->GetUInt32();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				}
			});
		}

		void
		ConsumeAndProduce::ToStream(
			IWriter*						aStream) const 
		{
			ToStreamBase(aStream);
			aStream->WriteUInt(m_consumeSourceItemId);
			aStream->WriteUInt(m_consumeSourceItemCount);
			aStream->WriteUInt(m_produceSourceItemId);
			aStream->WriteUInt(m_produceSourceItemCount);
		}

		bool
		ConsumeAndProduce::FromStream(
			IReader*						aStream) 
		{
			if (!FromStreamBase(aStream))
				return false;
			if (!aStream->ReadUInt(m_consumeSourceItemId))
				return false;
			if (!aStream->ReadUInt(m_consumeSourceItemCount))
				return false;
			if (!aStream->ReadUInt(m_produceSourceItemId))
				return false;
			if (!aStream->ReadUInt(m_produceSourceItemCount))
				return false;
			return true;
		}

		DirectEffectBase::Result
		ConsumeAndProduce::Resolve(
			int32_t							/*aTick*/,
			std::mt19937&					/*aRandom*/,
			const Manifest*					aManifest,
			CombatEvent::Id					/*aId*/,
			uint32_t						/*aAbilityId*/,
			const SourceEntityInstance&		/*aSourceEntityInstance*/,
			EntityInstance*					aSource,
			EntityInstance*					/*aTarget*/,
			const Vec2&						/*aAOETarget*/,
			const ItemInstanceReference&	/*aItem*/,
			IResourceChangeQueue*			/*aCombatResultQueue*/,
			IAuraEventQueue*				/*aAuraEventQueue*/,
			IEventQueue*					/*aEventQueue*/,
			const IWorldView*				/*aWorldView*/)
		{
			if(m_consumeSourceItemId != 0 || m_produceSourceItemId != 0)
			{
				Components::Inventory* inventory = aSource != NULL ? aSource->GetComponent<Components::Inventory>() : NULL;
				if(inventory != NULL)
				{		
					ItemList itemList = inventory->m_itemList;

					bool ok = false;

					if(m_consumeSourceItemId != 0)
					{
						if (itemList.HasItems(m_consumeSourceItemId, m_consumeSourceItemCount))
						{
							itemList.RemoveItems(m_consumeSourceItemId, m_consumeSourceItemCount, inventory->m_size);

							if(m_produceSourceItemId != 0)
								ok = itemList.AddMultipleToInventory(m_produceSourceItemId, m_produceSourceItemCount, aManifest->GetById<Data::Item>(m_produceSourceItemId), false, inventory->m_size);
						}
					}
					else if(m_produceSourceItemId != 0)
					{
						ok = itemList.AddMultipleToInventory(m_produceSourceItemId, m_produceSourceItemCount, aManifest->GetById<Data::Item>(m_produceSourceItemId), false, inventory->m_size);
					}

					if (ok)
					{
						inventory->UpdateItemList(itemList);
						inventory->SetPendingPersistenceUpdate(ComponentBase::PENDING_PERSISTENCE_UPDATE_HIGH_PRIORITY);
					}
					else
					{
						Components::PlayerPrivate* playerPrivate = aSource->GetComponent<Components::PlayerPrivate>();
						playerPrivate->m_errorNotification = ErrorNotification::ID_COULD_NOT_DO_THAT;
					}
				}
			}
			return Result();
		}

	}

}