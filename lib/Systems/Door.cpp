#include "../Pcheader.h"

#include <tpublic/Components/Openable.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Systems/Door.h>

#include <tpublic/IWorldView.h>

namespace tpublic::Systems
{

	Door::Door(
		const SystemData*	aData)
		: SystemBase(aData, 2)
	{
		RequireComponent<Components::Openable>();
		RequireComponent<Components::Position>();
	}
	
	Door::~Door()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	Door::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				aTicksInState,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		Components::Position* position = GetComponent<Components::Position>(aComponents);
		bool shouldBlock = aEntityState != EntityState::ID_DEAD;
		if(shouldBlock != position->IsBlocking())
		{
			if(shouldBlock)
				position->SetBlocking();
			else
				position->ClearBlocking();
			position->SetDirty();
		}

		{
			// Doors and barriers controlled by an encounter: closed (default) while it's active, open (dead) otherwise. With transition
			// ticks they go through spawning (closing) and despawning (opening) first, so they can be animated.
			const Components::Openable* openable = GetComponent<Components::Openable>(aComponents);
			if(openable->m_encounterId != 0)
			{
				bool active = aContext->m_worldView->WorldViewIsEncounterActive(openable->m_encounterId);
				bool transition = openable->m_encounterTransitionTicks > 0;

				if(active)
				{
					switch(aEntityState)
					{
					case EntityState::ID_DEAD:
					case EntityState::ID_DESPAWNING:
						return transition ? EntityState::ID_SPAWNING : EntityState::ID_DEFAULT;

					case EntityState::ID_SPAWNING:
						return aTicksInState >= openable->m_encounterTransitionTicks ? EntityState::ID_DEFAULT : EntityState::CONTINUE;

					default:
						return EntityState::CONTINUE;
					}
				}

				switch(aEntityState)
				{
				case EntityState::ID_SPAWNING:
					return EntityState::ID_DEAD; // Also when first spawned on the map, so it doesn't start by opening

				case EntityState::ID_DEFAULT:
					return transition ? EntityState::ID_DESPAWNING : EntityState::ID_DEAD;

				case EntityState::ID_DESPAWNING:
					return aTicksInState >= openable->m_encounterTransitionTicks ? EntityState::ID_DEAD : EntityState::CONTINUE;

				default:
					return EntityState::CONTINUE;
				}
			}
		}

		switch(aEntityState)
		{
		case EntityState::ID_SPAWNING:
			return EntityState::ID_DEFAULT;

		case EntityState::ID_DEFAULT:
			{
				Components::Openable* openable = GetComponent<Components::Openable>(aComponents);
				if (openable->m_opened)
					return EntityState::ID_DEAD;
			}
			break;

		case EntityState::ID_DEAD:
			if(aTicksInState > 30)				
			{
				Components::Openable* openable = GetComponent<Components::Openable>(aComponents);
				openable->m_opened = false;
				return EntityState::ID_DEFAULT;
			}
			break;

		default:
			break;
		}

		return EntityState::CONTINUE;
	}

}