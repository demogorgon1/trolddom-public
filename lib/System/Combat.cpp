#include "../Pcheader.h"

#include <kpublic/Components/Auras.h>
#include <kpublic/Components/CombatPublic.h>
#include <kpublic/Components/VisibleAuras.h>

#include <kpublic/Systems/Combat.h>

#include <kpublic/EntityInstance.h>
#include <kpublic/Resource.h>
#include <kpublic/Helpers.h>

namespace kpublic::Systems
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
	Combat::Update(
		uint32_t			/*aEntityInstanceId*/,
		EntityState::Id		aEntityState,
		ComponentBase**		aComponents,
		Context*			aContext) 
	{
		{
			Components::CombatPublic* combatPublic = GetComponent<Components::CombatPublic>(aComponents);

			if (aEntityState != EntityState::ID_DEAD)
			{
				for (Components::CombatPublic::Resource& resource : combatPublic->m_resources)
				{
					switch (resource.m_id)
					{
					case Resource::ID_HEALTH:
						if (aEntityState != EntityState::ID_IN_COMBAT && resource.m_current < resource.m_max)
							resource.m_current++;
						break;

					case Resource::ID_MANA:
						if (resource.m_current < resource.m_max)
							resource.m_current++;
						break;

					case Resource::ID_RAGE:
						if (aEntityState != EntityState::ID_IN_COMBAT && resource.m_current > 0)
							resource.m_current--;
						break;

					case Resource::ID_ENERGY:
						if (resource.m_current < resource.m_max)
							resource.m_current++;
						break;

					default:
						break;
					}
				}
			}
		}

		{
			Components::Auras* auras = GetComponent<Components::Auras>(aComponents);
			Components::VisibleAuras* visibleAuras = GetComponent<Components::VisibleAuras>(aComponents);

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
				}
			}

			if(auras->m_dirty)
			{
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

		return EntityState::CONTINUE;
	}

}