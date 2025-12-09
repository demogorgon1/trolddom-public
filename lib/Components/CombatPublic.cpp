#include "../Pcheader.h"

#include <tpublic/Components/CombatPublic.h>

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/AbilityModifier.h>

namespace tpublic::Components
{

	void
	CombatPublic::AddResourceMax(
		uint32_t				aResourceId,
		uint32_t				aValue)
	{
		for(ResourceEntry& t : m_resources)
		{
			if(t.m_id == aResourceId)
			{
				t.m_max += aValue;
				return;
			}
		}
		m_resources.push_back({ aResourceId, 0, aValue });
	}

	uint32_t
	CombatPublic::GetResourcePercentage(
		uint32_t				aResourceId) const
	{
		for (const ResourceEntry& t : m_resources)
		{
			if(t.m_id == aResourceId && t.m_max > 0)
				return (100 * t.m_current) / t.m_max;
		}
		return 0;
	}

	uint32_t
	CombatPublic::GetResource(
		uint32_t				aResourceId,
		uint32_t*				aOutMax) const
	{
		for (const ResourceEntry& t : m_resources)
		{
			if (t.m_id == aResourceId)
			{
				if(aOutMax != NULL)
					*aOutMax = t.m_max;
						
				return t.m_current;
			}
		}
		return 0;
	}

	uint32_t
	CombatPublic::GetResourceMax(
		uint32_t				aResourceId) const
	{
		for (const ResourceEntry& t : m_resources)
		{
			if (t.m_id == aResourceId)
				return t.m_max;
		}
		return 0;
	}

	CombatPublic::ResourceEntry*
	CombatPublic::GetResourceEntry(
		uint32_t				aResourceId)
	{
		for (ResourceEntry& t : m_resources)
		{
			if (t.m_id == aResourceId)
				return &t;
		}
		return NULL;
	}

	const CombatPublic::ResourceEntry*
	CombatPublic::GetResourceEntry(
		uint32_t				aResourceId) const
	{
		for (const ResourceEntry& t : m_resources)
		{
			if (t.m_id == aResourceId)
				return &t;
		}
		return NULL;
	}

	void
	CombatPublic::SetResourceToMax(
		uint32_t				aResourceId)
	{
		for (ResourceEntry& t : m_resources)
		{
			if (t.m_id == aResourceId)
			{
				t.m_current = t.m_max;
				return;
			}
		}
	}

	bool
	CombatPublic::GetResourceIndex(
		uint32_t				aResourceId,
		size_t&					aOut) const
	{
		size_t i = 0;
		for (const ResourceEntry& t : m_resources)
		{
			if (t.m_id == aResourceId)
			{
				aOut = i;
				return true;
			}
			i++;
		}
		return false;
	}

	bool
	CombatPublic::GetSecondaryResourceIndex(
		size_t&					aOut) const
	{
		// Secondary resource is the first one that's not health
		size_t i = 0;
		for (const ResourceEntry& t : m_resources)
		{
			if (t.m_id != Resource::ID_HEALTH)
			{
				aOut = i;
				return true;
			}
			i++;
		}
		return false;
	}

	void
	CombatPublic::SetResource(
		uint32_t				aResourceId,
		uint32_t				aValue)
	{
		for (ResourceEntry& t : m_resources)
		{
			if (t.m_id == aResourceId)
			{
				t.m_current = aValue;
				return;
			}
		}
	}

	bool
	CombatPublic::HasResourcesForAbility(
		const Data::Ability*								aAbility,
		const std::vector<const Data::AbilityModifier*>*	aModifiers,
		uint32_t											aBaseMana,
		float												aCostMultiplier) const
	{
		for(uint32_t resourceId = 1; resourceId < (uint32_t)Resource::NUM_IDS; resourceId++)
		{
			int32_t cost = (int32_t)aAbility->m_resourceCosts[resourceId];

			if(aModifiers != NULL && cost != 0)
			{
				for (const Data::AbilityModifier* modifier : *aModifiers)
				{
					if (modifier->m_modifyResourceCost.has_value() && modifier->m_modifyResourceCost->m_resourceId == resourceId)
						cost += modifier->m_modifyResourceCost->m_value;
				}						
			}

			if (resourceId == (uint32_t)Resource::ID_MANA)
			{
				// Mana cost is always as percentage of base mana
				cost = ((int32_t)aBaseMana * cost) / 100;
			}

			if(cost > 0 && aCostMultiplier != 1.0f)
				cost = (int32_t)((float)cost * aCostMultiplier);

			if(cost > 0 && (uint32_t)cost > GetResource(resourceId))
				return false;
		}
		return true;
	}

	bool
	CombatPublic::SubtractResourcesForAbility(
		const Data::Ability*								aAbility,
		const std::vector<const Data::AbilityModifier*>*	aModifiers,
		uint32_t											aBaseMana,
		float												aCostMultiplier)
	{
		for (uint32_t resourceId = 1; resourceId < (uint32_t)Resource::NUM_IDS; resourceId++)
		{
			uint32_t cost = aAbility->m_resourceCosts[resourceId];

			if(aModifiers != NULL && cost != 0)
			{
				for (const Data::AbilityModifier* modifier : *aModifiers)
				{
					if (modifier->m_modifyResourceCost.has_value() && modifier->m_modifyResourceCost->m_resourceId == resourceId)
						cost += modifier->m_modifyResourceCost->m_value;
				}						
			}

			if (resourceId == (uint32_t)Resource::ID_MANA)
			{
				// Mana cost is always as percentage of base mana
				cost = ((int32_t)aBaseMana * cost) / 100;
			}

			if (cost > 0 && aCostMultiplier != 1.0f)
				cost = (int32_t)((float)cost * aCostMultiplier);

			if(cost > 0)
			{
				size_t index;
				if(!GetResourceIndex(resourceId, index))
					return false;

				if(m_resources[index].m_current < cost)
					return false;

				m_resources[index].m_current -= cost;						
			}				
		}

		return true;
	}

	void
	CombatPublic::SetResurrectResources()
	{				
		for (ResourceEntry& t : m_resources)
		{
			const Resource::Info* resourceInfo = Resource::GetInfo((Resource::Id)t.m_id);
			t.m_current = resourceInfo->m_resurrectValue;
		}
	}

	void
	CombatPublic::SetDefaultResources()
	{
		for (ResourceEntry& t : m_resources)
		{
			const Resource::Info* resourceInfo = Resource::GetInfo((Resource::Id)t.m_id);

			if(resourceInfo->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
				t.m_current = t.m_max;
			else
				t.m_current = 0;
		}
	}

	void
	CombatPublic::SetLevelUpResources()
	{
		for (ResourceEntry& t : m_resources)
		{
			const Resource::Info* resourceInfo = Resource::GetInfo((Resource::Id)t.m_id);

			if(resourceInfo->m_flags & Resource::FLAG_DEFAULT_TO_MAX)
				t.m_current = t.m_max;
		}
	}

	bool
	CombatPublic::SetZeroResources()
	{
		bool changed = false;
		for (ResourceEntry& t : m_resources)
		{
			if(t.m_current > 0)
			{
				changed = true;
				t.m_current = 0;
			}
		}
		return changed;
	}

	bool
	CombatPublic::IsOneOfCreatureTypes(
		const std::vector<uint32_t>&	aCreatureTypeIds) const
	{
		for(uint32_t id : aCreatureTypeIds)
		{
			if(m_creatureTypeId == id)
				return true;
		}
		return false;
	}

}