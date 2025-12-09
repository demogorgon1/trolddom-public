#include "../Pcheader.h"

#include <tpublic/Components/Environment.h>
#include <tpublic/Components/Owner.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Data/Ability.h>

#include <tpublic/Systems/Environment.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IEventQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>

namespace tpublic::Systems
{

	Environment::Environment(
		const SystemData*	aData)
		: SystemBase(aData)
	{
		RequireComponent<Components::Environment>();
		RequireComponent<Components::Position>();
		RequireComponent<Components::Owner>();
	}
	
	Environment::~Environment()
	{

	}

	//---------------------------------------------------------------------------

	void			
	Environment::Init(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		ComponentBase**		aComponents,
		int32_t				aTick) 
	{
		Components::Environment* environment = GetComponent<Components::Environment>(aComponents);
		assert(environment != NULL);

		environment->m_despawnTick = aTick + environment->m_duration;
	}

	EntityState::Id
	Environment::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			aEntityInstanceId,
		EntityState::Id		aEntityState,
		int32_t				aTicksInState,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		Components::Environment* environment = GetComponent<Components::Environment>(aComponents);
		assert(environment != NULL);

		if(aEntityState == EntityState::ID_DESPAWNED)
			return EntityState::CONTINUE;

		if (aEntityState == EntityState::ID_SPAWNING)
			return aTicksInState < SPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DEFAULT;

		if (aEntityState == EntityState::ID_DESPAWNING)
			return aTicksInState < DESPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DESPAWNED;

		if(aContext->m_tick > environment->m_despawnTick && environment->m_duration != 0)
			return EntityState::ID_DESPAWNING;

		const Components::Position* position = GetComponent<Components::Position>(aComponents);
		assert(position != NULL);

		const Components::Owner* owner = GetComponent<Components::Owner>(aComponents);		

		int32_t ticksSinceLastUpdate = aContext->m_tick - environment->m_lastUpdateTick;

		if (ticksSinceLastUpdate >= environment->m_tickInterval && environment->m_abilityId != 0 && owner != NULL)
		{			
			const Data::Ability* ability = GetManifest()->GetById<Data::Ability>(environment->m_abilityId);

			SourceEntityInstance sourceEntityInstance = owner->m_ownerSourceEntityInstance.IsSet() ? owner->m_ownerSourceEntityInstance : SourceEntityInstance{ aEntityInstanceId, 0 };
			
			if(ability->TargetAOE())
			{
				// This is a self targeted AOE ability
				aContext->m_eventQueue->EventQueueAbility(
					sourceEntityInstance,
					aEntityInstanceId, 
					position->m_position, 
					ability, 
					ItemInstanceReference(), 
					NULL,
					position->m_position);
			}
			else
			{
				uint32_t ownerEntityInstanceId = owner != NULL ? owner->m_ownerSourceEntityInstance.m_entityInstanceId : 0;

				// For all other abilities just target it at whatever other entity at the same position
				aContext->m_worldView->WorldViewEntityInstancesAtPosition(position->m_position, [&](
					const EntityInstance* aEntityInstance)
				{
					if(aEntityInstance->GetEntityInstanceId() != aEntityInstanceId && aEntityInstance->GetEntityInstanceId() != ownerEntityInstanceId)
						aContext->m_eventQueue->EventQueueAbility(sourceEntityInstance, aEntityInstance->GetEntityInstanceId(), position->m_position, ability, ItemInstanceReference(), NULL, position->m_position);

					return false; // Don't stop
				});
			}

			environment->m_lastUpdateTick = aContext->m_tick;
		}

		return EntityState::CONTINUE;
	}

}