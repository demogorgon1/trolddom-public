#include "../Pcheader.h"

#include <tpublic/Components/CombatPrivate.h>
#include <tpublic/Components/CombatPublic.h>

#include <tpublic/Systems/ManaRegen.h>

namespace tpublic::Systems
{

	ManaRegen::ManaRegen(
		const SystemData*	aData)
		: SystemBase(aData)
	{
		RequireComponent<Components::CombatPrivate>();
		RequireComponent<Components::CombatPublic>();
	}
	
	ManaRegen::~ManaRegen()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	ManaRegen::UpdatePrivate(
		uint32_t			/*aEntityId*/,
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		int32_t				/*aTicksInState*/,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

		size_t i;
		if (combatPublic->GetResourceIndex(Resource::ID_MANA, i))
		{
			Components::CombatPublic::ResourceEntry& resource = combatPublic->m_resources[i];
			if(resource.m_current < resource.m_max)
			{
				const Components::CombatPrivate* combatPrivate = GetComponent<Components::CombatPrivate>(aComponents);

				// 5-second rule
				if (aContext->m_tick - combatPublic->m_lastCastingTick >= 50 &&
					aContext->m_tick - combatPublic->m_last5SecRuleManaRegenTick >= 20 && 
					combatPrivate->m_manaRegenerationNotCastingPer5Sec > 0)
				{
					uint32_t regenValue = (combatPrivate->m_manaRegenerationNotCastingPer5Sec * 20) / 50;

					if(regenValue == 0)
						regenValue = 1;

					resource.m_current += regenValue;
					if (resource.m_current > resource.m_max)
						resource.m_current = resource.m_max;

					combatPublic->m_last5SecRuleManaRegenTick = aContext->m_tick;
					combatPublic->SetPendingPersistenceUpdate(ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY);
				}

				// Base mana regen
				if(aContext->m_tick - combatPublic->m_lastBaseManaRegenTick >= 20 && combatPrivate->m_manaRegenerationPer5Sec > 0)
				{
					uint32_t regenValue = (combatPrivate->m_manaRegenerationPer5Sec * 20) / 50;

					if(regenValue == 0)
						regenValue = 1;

					resource.m_current += regenValue;
					if (resource.m_current > resource.m_max)
						resource.m_current = resource.m_max;

					combatPublic->m_lastBaseManaRegenTick = aContext->m_tick;
					combatPublic->SetPendingPersistenceUpdate(ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY);
				}
			}
		}

		return EntityState::CONTINUE;
	}

}