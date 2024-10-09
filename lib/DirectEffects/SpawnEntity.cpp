#include "../Pcheader.h"

#include <tpublic/Components/DisplayName.h>
#include <tpublic/Components/GuildName.h>
#include <tpublic/Components/MinionPrivate.h>
#include <tpublic/Components/MinionPublic.h>
#include <tpublic/Components/NPC.h>
#include <tpublic/Components/Owner.h>
#include <tpublic/Components/PlayerPublic.h>
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
			else if (aChild->m_name == "map_entity_spawn")
				m_mapEntitySpawnId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_ENTITY_SPAWN, aChild->GetIdentifier());
			else if(aChild->m_name == "spawn_flags")
				m_spawnFlags = SourceToSpawnFlags(aChild);
			else if(aChild->m_name == "npc_target_threat")
				m_npcTargetThreat = aChild->GetInt32();
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
		});
	}

	void
	SpawnEntity::ToStream(
		IWriter*						aStream) const 
	{	
		ToStreamBase(aStream);
		aStream->WriteUInt(m_mapEntitySpawnId);
		aStream->WriteUInt(m_entityId);
		aStream->WritePOD(m_spawnFlags);
		aStream->WriteInt(m_npcTargetThreat);
	}

	bool
	SpawnEntity::FromStream(
		IReader*						aStream) 
	{
		if(!FromStreamBase(aStream))
			return false;
		if (!aStream->ReadUInt(m_mapEntitySpawnId))
			return false;
		if (!aStream->ReadUInt(m_entityId))
			return false;
		if (!aStream->ReadPOD(m_spawnFlags))
			return false;
		if (!aStream->ReadInt(m_npcTargetThreat))
			return false;
		return true;
	}

	DirectEffectBase::Result
	SpawnEntity::Resolve(
		int32_t							/*aTick*/,
		std::mt19937&					/*aRandom*/,
		const Manifest*					/*aManifest*/,
		CombatEvent::Id					/*aId*/,
		uint32_t						/*aAbilityId*/,
		const SourceEntityInstance&		/*aSourceEntityInstance*/,
		EntityInstance*					aSource,
		EntityInstance*					aTarget,
		const Vec2&						/*aAOETarget*/,
		const ItemInstanceReference&	/*aItem*/,
		IResourceChangeQueue*			/*aCombatResultQueue*/,
		IAuraEventQueue*				/*aAuraEventQueue*/,
		IEventQueue*					aEventQueue,
		const IWorldView*				/*aWorldView*/) 
	{					
		EntityInstance* spawnedEntity = aEventQueue->EventQueueSpawnEntity(m_entityId, EntityState::ID_DEFAULT, m_mapEntitySpawnId, false);

		if(m_mapEntitySpawnId == 0)
		{
			// Position
			Components::Position* spawnedEntityPosition = spawnedEntity->GetComponent<Components::Position>();

			if (m_spawnFlags & SPAWN_FLAG_AT_TARGET)
				spawnedEntityPosition->m_position = aTarget->GetComponent<Components::Position>()->m_position;
			else
				spawnedEntityPosition->m_position = aSource->GetComponent<Components::Position>()->m_position;
		}

		if((m_spawnFlags & SPAWN_FLAG_NO_OWNER) == 0)
		{
			// Owner
			Components::Owner* spawnedEntityOwner = spawnedEntity->GetComponent<Components::Owner>();
			spawnedEntityOwner->m_ownerSourceEntityInstance = { aSource->GetEntityInstanceId(), aSource->GetSeq() };
		}

		if(m_npcTargetThreat != 0)
		{
			// Initial NPC threat on target
			Components::NPC* npc = spawnedEntity->GetComponent<Components::NPC>();
			npc->m_spawnWithTarget = { { aTarget->GetEntityInstanceId(), aTarget->GetSeq() }, m_npcTargetThreat };
		}

		// Minion?
		{
			Components::MinionPublic* minionPublic = spawnedEntity->GetComponent<Components::MinionPublic>();
			if(minionPublic != NULL)
			{
				minionPublic->m_ownerEntityInstanceId = aSource->GetEntityInstanceId();
				minionPublic->SetDirty();
			}

			Components::MinionPrivate* minionPrivate = spawnedEntity->GetComponent<Components::MinionPrivate>();
			if (minionPrivate != NULL && aSource->IsPlayer())
			{
				const Components::PlayerPublic* playerPublic = aSource->GetComponent<Components::PlayerPublic>();

				if(minionPrivate->m_seed == 0) // No minion seed, derive one from the spawner's character id
					minionPrivate->m_seed = (uint32_t)Hash::Splitmix_2_32(playerPublic->m_characterId, 0);

				minionPrivate->SetDirty();

				// Guild name should show owner
				const Components::DisplayName* displayName = aSource->GetComponent<Components::DisplayName>();
				Components::GuildName* guildName = spawnedEntity->GetComponent<Components::GuildName>();
				guildName->m_string = displayName->m_string + "'s Minion";
				guildName->SetDirty();
			}
		}

		return Result();
	}

}