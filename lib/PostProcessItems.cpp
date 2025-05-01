#include "Pcheader.h"

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/DialogueScreen.h>
#include <tpublic/Data/Item.h>

#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace PostProcessItems
	{

		void		
		Run(
			Manifest* aManifest)
		{
			// First identify all items that can be bought from vendors
			std::unordered_set<uint32_t> vendorItemIds;

			aManifest->GetContainer<Data::DialogueScreen>()->ForEach([&vendorItemIds](
				const Data::DialogueScreen* aDialogueScreen)
			{
				for(const Data::DialogueScreen::Sell& sell : aDialogueScreen->m_sell)
					vendorItemIds.insert(sell.m_itemId);
				return true;
			});

			// For each item
			{
				// FIXME: some of these parameters should be from the manifest
				WeightedRandom<ArmorStyle::Id> armorStyles;
				armorStyles.AddPossibility(2, ArmorStyle::ID_BROWN);
				armorStyles.AddPossibility(2, ArmorStyle::ID_GREEN);
				armorStyles.AddPossibility(2, ArmorStyle::ID_CYAN);
				armorStyles.AddPossibility(1, ArmorStyle::ID_RED);
				armorStyles.AddPossibility(1, ArmorStyle::ID_PURPLE);

				aManifest->GetContainer<Data::Item>()->ForEach([&armorStyles, &vendorItemIds, aManifest](
					Data::Item* aItem)
				{
					// Automatically set vendor flag if can be bought from a vendor
					if(vendorItemIds.contains(aItem->m_id))
						aItem->m_flags |= Data::Item::FLAG_VENDOR;

					// If no item level (or required level), set item level based on use ability if any
					if(aItem->m_itemLevel == 0 && aItem->m_useAbilityId != 0)
					{
						const Data::Ability* ability = aManifest->GetById<Data::Ability>(aItem->m_useAbilityId);
						if(ability->m_requiredLevel != 0)
							aItem->m_itemLevel = ability->m_requiredLevel;
					}

					// Chest armor needs visuals
					if (!aItem->m_armorStyleVisual && aItem->IsEquippableInSlot(EquipmentSlot::ID_CHEST))
					{
						std::mt19937 random((uint32_t)Hash::Splitmix_64((uint64_t)aItem->m_id));

						uint32_t r = Helpers::RandomInRange<uint32_t>(random, 0, 255);
						uint32_t g = Helpers::RandomInRange<uint32_t>(random, 0, 255);
						uint32_t b = Helpers::RandomInRange<uint32_t>(random, 0, 255);
						uint32_t a = Helpers::RandomInRange<uint32_t>(random, 0, 255);

						// Don't make it too bright
						if ((r + g + b) / 3 > 150)
							a /= 2;

						ArmorStyle::Visual armorStyleVisual;
						armorStyleVisual.m_decorationColor.m_r = (uint8_t)r;
						armorStyleVisual.m_decorationColor.m_g = (uint8_t)g;
						armorStyleVisual.m_decorationColor.m_b = (uint8_t)b;
						armorStyleVisual.m_decorationColor.m_a = (uint8_t)a;
						armorStyleVisual.m_id = armorStyles.Get(random);
						aItem->m_armorStyleVisual = armorStyleVisual;
					}
					return true;
				});
			}
		}

	}

}