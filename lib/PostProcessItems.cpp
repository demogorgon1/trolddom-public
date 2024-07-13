#include "Pcheader.h"

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
			// FIXME: some of these parameters should be from the manifest
			WeightedRandom<ArmorStyle::Id> armorStyles;
			armorStyles.AddPossibility(2, ArmorStyle::ID_BROWN);
			armorStyles.AddPossibility(2, ArmorStyle::ID_GREEN);
			armorStyles.AddPossibility(2, ArmorStyle::ID_CYAN);
			armorStyles.AddPossibility(1, ArmorStyle::ID_RED);
			armorStyles.AddPossibility(1, ArmorStyle::ID_PURPLE);

			aManifest->GetContainer<Data::Item>()->ForEach([&armorStyles](
				Data::Item* aItem)
			{
				// Chest armor needs visuals
				if(!aItem->m_armorStyleVisual && aItem->IsEquippableInSlot(EquipmentSlot::ID_CHEST))
				{
					std::mt19937 random((uint32_t)Hash::Splitmix_64((uint64_t)aItem->m_id));

					uint32_t r = Helpers::RandomInRange<uint32_t>(random, 0, 255);
					uint32_t g = Helpers::RandomInRange<uint32_t>(random, 0, 255);
					uint32_t b = Helpers::RandomInRange<uint32_t>(random, 0, 255);
					uint32_t a = Helpers::RandomInRange<uint32_t>(random, 0, 255);

					// Don't make it too bright
					if((r + g + b) / 3 > 150)
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