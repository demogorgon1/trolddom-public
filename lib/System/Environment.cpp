#include "../Pcheader.h"

#include <tpublic/Components/Environment.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Systems/Environment.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/IWorldView.h>

namespace tpublic::Systems
{

	Environment::Environment(
		const Manifest*		aManifest)
		: SystemBase(aManifest)
	{
		RequireComponent<Components::Environment>();
		RequireComponent<Components::Position>();
	}
	
	Environment::~Environment()
	{

	}

	//---------------------------------------------------------------------------

	void			
	Environment::Init(
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		ComponentBase**		aComponents,
		int32_t				aTick) 
	{
		Components::Environment* environment = GetComponent<Components::Environment>(aComponents);
		assert(environment != NULL);

		environment->m_destroyTick = aTick + environment->m_duration;
	}

	EntityState::Id
	Environment::UpdatePrivate(
		uint32_t			aEntityInstanceId,
		EntityState::Id		/*aEntityState*/,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		Components::Environment* environment = GetComponent<Components::Environment>(aComponents);
		assert(environment != NULL);

		if(aContext->m_tick > environment->m_destroyTick)
			return EntityState::DESTROY;

		const Components::Position* position = GetComponent<Components::Position>(aComponents);
		assert(position != NULL);

		int32_t ticksSinceLastUpdate = aContext->m_tick - environment->m_lastUpdateTick;

		if (ticksSinceLastUpdate >= environment->m_tickInterval)
		{
			aContext->m_worldView->QueryEntityInstancesAtPosition(position->m_position, [&](
				const EntityInstance* aEntityInstance)
			{
				if(aEntityInstance->GetEntityInstanceId() != aEntityInstanceId)
				{
					printf("tick: %u\n", aEntityInstance->GetEntityInstanceId());
				}

				return false; // Don't stop
			});

			environment->m_lastUpdateTick = aContext->m_tick;
		}

		//return EntityState::CONTINUE;
		return EntityState::ID_DEAD;
	}

}