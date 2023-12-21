#include "../Pcheader.h"

#include <tpublic/Components/Environment.h>
#include <tpublic/Components/Owner.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Systems/Environment.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IAbilityQueue.h>
#include <tpublic/IWorldView.h>
#include <tpublic/Manifest.h>

namespace tpublic::Systems
{

	Environment::Environment(
		const Manifest*		aManifest)
		: SystemBase(aManifest)
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

		if (aEntityState == EntityState::ID_SPAWNING)
			return aTicksInState < SPAWN_TICKS ? EntityState::CONTINUE : EntityState::ID_DEFAULT;

		if (aEntityState == EntityState::ID_DESPAWNING)
			return aTicksInState < DESPAWN_TICKS ? EntityState::CONTINUE : EntityState::DESTROY;

		if(aContext->m_tick > environment->m_despawnTick)
			return EntityState::ID_DESPAWNING;

		const Components::Position* position = GetComponent<Components::Position>(aComponents);
		assert(position != NULL);

		const Components::Owner* owner = GetComponent<Components::Owner>(aComponents);		
		assert(owner != NULL);

		int32_t ticksSinceLastUpdate = aContext->m_tick - environment->m_lastUpdateTick;

		if (ticksSinceLastUpdate >= environment->m_tickInterval && environment->m_abilityId != 0)
		{
			const Data::Ability* ability = GetManifest()->GetById<tpublic::Data::Ability>(environment->m_abilityId);

			aContext->m_worldView->QueryEntityInstancesAtPosition(position->m_position, [&](
				const EntityInstance* aEntityInstance)
			{
				if(aEntityInstance->GetEntityInstanceId() != aEntityInstanceId && aEntityInstance->GetEntityInstanceId() != owner->m_ownerEntityInstanceId)
					aContext->m_abilityQueue->AddAbility(aEntityInstanceId, aEntityInstance->GetEntityInstanceId(), tpublic::Vec2(), ability, NULL);

				return false; // Don't stop
			});

			environment->m_lastUpdateTick = aContext->m_tick;
		}

		return EntityState::CONTINUE;
	}

}