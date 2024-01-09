#include "../Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>

#include <tpublic/Systems/HealthRegen.h>

namespace tpublic::Systems
{

	HealthRegen::HealthRegen(
		const SystemData*	aData)
		: SystemBase(aData, UPDATE_INTERVAL)
	{
		RequireComponent<Components::CombatPrivate>();
		RequireComponent<Components::CombatPublic>();
	}
	
	HealthRegen::~HealthRegen()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	HealthRegen::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

		size_t i;
		if (combatPublic->GetResourceIndex(Resource::ID_HEALTH, i))
		{
			Components::CombatPublic::ResourceEntry& resource = combatPublic->m_resources[i];
			if (resource.m_current < resource.m_max)
			{
				const Components::CombatPrivate* combatPrivate = GetComponent<Components::CombatPrivate>(aComponents);

				uint32_t regenValue = 0;

				if(aEntityState == EntityState::ID_DEFAULT)
				{
					regenValue = (resource.m_max * (combatPrivate->m_healthRegenNotInCombat + 100)) / 10000;
					if(regenValue == 0)
						regenValue = 1;
				}
				else if (aEntityState == EntityState::ID_IN_COMBAT)
				{
					regenValue = (combatPrivate->m_healthRegenerationPer5Sec * UPDATE_INTERVAL) / 50;
				}

				if(regenValue > 0)
				{
					resource.m_current += regenValue;
					if (resource.m_current > resource.m_max)
						resource.m_current = resource.m_max;

					combatPublic->SetPendingPersistenceUpdate(ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY);
				}
			}
		}

		return EntityState::CONTINUE;
	}

}