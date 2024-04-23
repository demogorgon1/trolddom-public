#include "../Pcheader.h"

#include <tpublic/Components/Owner.h>
#include <tpublic/Components/Position.h>

#include <tpublic/DirectEffects/SpawnEntity.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/Manifest.h>

namespace tpublic::DirectEffects
{

	void
	SpawnEntity::FromSource(
		const SourceNode*				aSource) 
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "entity")
				m_entityId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ENTITY, aChild->GetIdentifier());
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
		});
	}

	void
	SpawnEntity::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);
		aStream->WriteUInt(m_entityId);
	}

	bool
	SpawnEntity::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if(!aStream->ReadUInt(m_entityId))
			return false;
		return true;
	}

	CombatEvent::Id
	SpawnEntity::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					aId,
		uint32_t						/*aAbilityId*/,
		EntityInstance*					aSource,
		EntityInstance*					/*aTarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{					
		EntityInstance* spawnedEntity = aEventQueue->EventQueueSpawnEntity(m_entityId, EntityState::ID_DEFAULT);

		// Position
		{
			const Components::Position* sourcePosition = aSource->GetComponent<Components::Position>();
			Components::Position* spawnedEntityPosition = spawnedEntity->GetComponent<Components::Position>();
			spawnedEntityPosition->m_position = sourcePosition->m_position;
		}

		// Owner
		{
			Components::Owner* spawnedEntityOwner = spawnedEntity->GetComponent<Components::Owner>();
			spawnedEntityOwner->m_ownerEntityInstanceId = aSource->GetEntityInstanceId();
		}

		return aId;
	}

}