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

			if(aAbility->m_debugInfo)
			{				
				for (const std::unique_ptr<Data::Ability::DirectEffectEntry>& directEffect : aAbility->m_directEffects)
					directEffect->m_directEffectBase->Validate(aAbility->m_debugInfo.value(), aManifest, aAbility);
			}
		}

	}

	namespace PostProcessAbilities
	{

		void		
		Run(
			Manifest* aManifest)
		{
			size_t errorCount = 0;

			{
				DataErrorHandling::ScopedErrorCallback errorCallback([&errorCount](
					const char* aError)
				{
					printf("\x1B[31mERROR: %s\x1B[37m\n", aError);
					errorCount++;

					throw int(0); // Need to abort error handling, otherwise the process will exit on first error
				});

				aManifest->GetContainer<Data::Ability>()->ForEach([aManifest](
					Data::Ability* aAbility)
				{
					try
					{
						_PostProcessAbility(aManifest, aAbility);
					}
					catch(int /*e*/)
					{
						// Do nothing
					}
					return true;
				});
			}


			TP_CHECK(errorCount == 0, "%zu error%s encountered while processing abilities.", errorCount, errorCount == 1 ? "" : "s");
		}

	}

}