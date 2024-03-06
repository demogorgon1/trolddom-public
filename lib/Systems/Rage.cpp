#include "../Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>

#include <tpublic/Systems/Rage.h>

namespace tpublic::Systems
{

	Rage::Rage(
		const SystemData*	aData)
		: SystemBase(aData, 20)
	{
		RequireComponent<Components::CombatPrivate>();
		RequireComponent<Components::CombatPublic>();
	}
	
	Rage::~Rage()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	Rage::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

		size_t i;
		if (combatPublic->GetResourceIndex(Resource::ID_RAGE, i))
		{
			Components::CombatPublic::ResourceEntry& resource = combatPublic->m_resources[i];
			const Components::CombatPrivate* combatPrivate = GetComponent<Components::CombatPrivate>(aComponents);

			switch(aEntityState)
			{
			case EntityState::ID_DEFAULT:
				if (resource.m_current > 0 && combatPrivate->m_rageDecayNotInCombatPer5Sec > 0)
				{
					uint32_t decayValue = (combatPrivate->m_rageDecayNotInCombatPer5Sec * 20) / 50;

					if (decayValue == 0)
						decayValue = 1;

					if (resource.m_current > decayValue)
						resource.m_current -= decayValue;
					else 
						resource.m_current = 0;

					combatPublic->SetPendingPersistenceUpdate(ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY);
				}
				break;

			case EntityState::ID_IN_COMBAT:
				if(resource.m_current < resource.m_max && combatPrivate->m_rageGenerationInCombatPer5Sec > 0)
				{
					uint32_t generateValue = (combatPrivate->m_rageGenerationInCombatPer5Sec * 20) / 50;

					if (generateValue == 0)
						generateValue = 1;

					resource.m_current += generateValue;
					if (resource.m_current > resource.m_max)
						resource.m_current = resource.m_max;

					combatPublic->SetPendingPersistenceUpdate(ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY);
				}
				break;

			default:
				break;
			}
		}

		return EntityState::CONTINUE;
	}

}