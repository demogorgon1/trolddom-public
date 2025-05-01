#include "Pcheader.h"

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/Item.h>

#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace
	{

		void
		_PostProcessAbility(
			const Manifest*			aManifest,
			Data::Ability*			aAbility)
		{
			// FIXME: should probably generalize this somehow if needed elsewhere
			switch(aAbility->m_iconFrom.m_type)
			{
			case DataType::INVALID_ID:		break;
			case DataType::ID_ITEM:			aAbility->m_iconSpriteId = aManifest->GetById<Data::Item>(aAbility->m_iconFrom.m_id)->m_iconSpriteId; break;
			default:						TP_VERIFY(false, aAbility->m_debugInfo, "Invalid data reference.");
			}

			switch (aAbility->m_descriptionFrom.m_type)
			{
			case DataType::INVALID_ID:		
				break;
			
			case DataType::ID_ITEM:			
				{ 
					const Data::Item* item = aManifest->GetById<Data::Item>(aAbility->m_descriptionFrom.m_id);
					if(item->m_useAbilityId != 0)
						aAbility->m_description = aManifest->GetById<Data::Ability>(item->m_useAbilityId)->m_description;
				}
				break; 
			
			default:						
				TP_VERIFY(false, aAbility->m_debugInfo, "Invalid data reference.");
				break;
			}
		}

	}

	namespace PostProcessAbilities
	{

		void		
		Run(
			Manifest* aManifest)
		{
			aManifest->GetContainer<Data::Ability>()->ForEach([aManifest](
				Data::Ability* aAbility)
			{
				_PostProcessAbility(aManifest, aAbility);
				return true;
			});
		}

	}

}