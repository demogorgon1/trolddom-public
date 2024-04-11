#include "../Pcheader.h"

#include <tpublic/Components/Openable.h>
#include <tpublic/Components/Position.h>

#include <tpublic/Systems/Door.h>

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
		Context*			/*aContext*/) 
	{
		Components::Position* position = GetComponent<Components::Position>(aComponents);
		bool shouldBlock = aEntityState != EntityState::ID_DEAD;
		if(shouldBlock != position->m_block)
		{
			position->m_block = shouldBlock;
			position->SetDirty();
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