#include "../Pcheader.h"

#include <tpublic/Components/Auras.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/VisibleAuras.h>

#include <tpublic/Systems/Combat.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Resource.h>
#include <tpublic/Helpers.h>

namespace tpublic::Systems
{

	Combat::Combat(
		const Manifest*		aManifest)
		: SystemBase(aManifest)
	{
		RequireComponent<Components::Auras>();
		RequireComponent<Components::CombatPublic>();
		RequireComponent<Components::VisibleAuras>();
	}
	
	Combat::~Combat()
	{

	}

	//---------------------------------------------------------------------------

	EntityState::Id
	Combat::UpdatePrivate(
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		/*aEntityState*/,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		{
			Components::Auras* auras = GetComponent<Components::Auras>(aComponents);

			for(size_t i = 0; i < auras->m_entries.size(); i++)
			{
				std::unique_ptr<Components::Auras::Entry>& entry = auras->m_entries[i];

				for(size_t j = 0; j < entry->m_effects.size(); j++)
				{
					std::unique_ptr<AuraEffectBase>& effect = entry->m_effects[j];

					if(!effect->Update(aContext->m_tick))
					{
						effect.reset();
						Helpers::RemoveCyclicFromVector(entry->m_effects, j);
						j--;
					}						
				}

				if(entry->m_effects.size() == 0 || aContext->m_tick >= entry->m_end)
				{
					entry.reset();
					Helpers::RemoveCyclicFromVector(auras->m_entries, i);
					i--;

					auras->m_dirty = true;
					auras->SetPendingPersistenceUpdate(tpublic::ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY);
				}
			}
		}

		return EntityState::CONTINUE;
	}

	void
	Combat::UpdatePublic(
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		ComponentBase**		aComponents,
		Context*			/*aContext*/) 
	{
		{
			Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

			if (aEntityState != EntityState::ID_DEAD)
			{
				bool dirty = false;

				for (Components::CombatPublic::Resource& resource : combatPublic->m_resources)
				{
					switch (resource.m_id)
					{
					case Resource::ID_HEALTH:
						if (aEntityState != EntityState::ID_IN_COMBAT && resource.m_current < resource.m_max)
						{
							resource.m_current++;
							dirty = true;
						}
						break;

					case Resource::ID_MANA:
						if (resource.m_current < resource.m_max)
						{
							resource.m_current++;
							dirty = true;
						}
						break;

					case Resource::ID_RAGE:
						if (aEntityState != EntityState::ID_IN_COMBAT && resource.m_current > 0)
						{
							resource.m_current--;
							dirty = true;
						}
						break;

					case Resource::ID_ENERGY:
						if (resource.m_current < resource.m_max)
						{
							resource.m_current++;
							dirty = true;
						}
						break;

					default:
						break;
					}
				}

				if(dirty)
				{
					combatPublic->SetPendingPersistenceUpdate(tpublic::ComponentBase::PENDING_PERSISTENCE_UPDATE_LOW_PRIORITY);
				}
			}
		}

		{
			Components::Auras* auras = GetComponent<Components::Auras>(aComponents);

			if(auras->m_dirty)
			{
				Components::VisibleAuras* visibleAuras = GetComponent<Components::VisibleAuras>(aComponents);
				visibleAuras->m_entries.clear();

				for (const std::unique_ptr<Components::Auras::Entry>& entry : auras->m_entries)
				{
					Components::VisibleAuras::Entry t;
					t.m_auraId = entry->m_auraId;
					t.m_entityInstanceId = entry->m_entityInstanceId;
					t.m_start = entry->m_start;
					t.m_end = entry->m_end;
					visibleAuras->m_entries.push_back(t);
				}

				auras->m_dirty = false;
			}
		}
	}

}