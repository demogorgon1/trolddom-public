#include "Pcheader.h"

#include <tpublic/Components/VisibleAuras.h>

#include <tpublic/Data/Ability.h>

#include <tpublic/EntityInstance.h>

namespace tpublic
{

	namespace AbilityRequirements
	{

		bool	
		Check(
			const Data::Ability*			aAbility,
			const EntityInstance*			aSelf,
			const EntityInstance*			aTarget)
		{
			for(const Data::Ability::Requirement& requirement : aAbility->m_requirements)
			{
				const EntityInstance* entity = NULL;
				switch (requirement.m_target)
				{
				case Data::Ability::Requirement::TARGET_SELF:	entity = aSelf; break;
				case Data::Ability::Requirement::TARGET_TARGET: entity = aTarget; break;
				default: assert(false); break;
				}

				if (entity == NULL)
					return false;

				switch(requirement.m_type)
				{
				case Data::Ability::Requirement::TYPE_MUST_HAVE_AURA:
				case Data::Ability::Requirement::TYPE_MUST_NOT_HAVE_AURA:
					{
						const Components::VisibleAuras* visibleAuras = entity->GetComponent<Components::VisibleAuras>();
						bool hasAura = visibleAuras->HasAura(requirement.m_id);
						if (requirement.m_type == Data::Ability::Requirement::TYPE_MUST_HAVE_AURA && !hasAura)
							return false;
						else if (requirement.m_type == Data::Ability::Requirement::TYPE_MUST_NOT_HAVE_AURA && hasAura)
							return false;
					}
					break;					

				default:
					assert(false);
					break;
				}
			}

			return true;
		}

	}

}