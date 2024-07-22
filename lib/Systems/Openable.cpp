#include "../Pcheader.h"

#include <tpublic/Components/Lootable.h>
#include <tpublic/Components/Openable.h>

#include <tpublic/Systems/Openable.h>

#include <tpublic/IWorldView.h>
#include <tpublic/MapData.h>

namespace tpublic::Systems
{

	Openable::Openable(
		const SystemData*	aData)
		: SystemBase(aData, UPDATE_INTERVAL)
	{
		RequireComponent<Components::Openable>();
	}
	
	Openable::~Openable()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	Openable::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				aTicksInState,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		Components::Openable* openable = GetComponent<Components::Openable>(aComponents);

		if(aEntityState == EntityState::ID_DEAD)
		{
			if(aContext->m_worldView->WorldViewGetMapData()->m_type == MapType::ID_OPEN_WORLD && aTicksInState > openable->m_deadDespawnTicks)
				return EntityState::ID_DESPAWNED;

			return EntityState::CONTINUE;
		}

		if(openable->m_duration != 0 && aTicksInState >= openable->m_duration)
			return EntityState::ID_DESPAWNED;

		if(openable->m_opened)
		{
			if(openable->m_despawn)
				return EntityState::ID_DESPAWNED;

			if (openable->m_kill)
				return EntityState::ID_DEAD;

			openable->m_opened = false;
		}

		return EntityState::CONTINUE;
	}

}