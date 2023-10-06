#pragma once

#include "Data/Ability.h"
#include "Data/Item.h"

#include "ItemInstance.h"
#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class ItemList
	{
	public:
		struct Entry
		{	
			void
			ToStream(
				IWriter*								aStream) const 
			{
				aStream->WritePOD(m_item);
			}

			bool
			FromStream(
				IReader*								aStream) 
			{
				if (!aStream->ReadPOD(m_item))
					return false;
				return true;
			}

			// Public data
			ItemInstance				m_item;
		};

		void				ToStream(
								IWriter*									aWriter) const;
		bool				FromStream(
								IReader*									aReader);
		bool				AddToInventory(
								const ItemInstance&							aItemInstance,
								const Data::Item*							aItemData);
		bool				CanAddMultipleToInventory(
								uint32_t									aItemId,
								uint32_t									aQuantity,
								const Data::Item*							aItemData) const;
		void				AddMultipleToInventory(
								uint32_t									aItemId,
								uint32_t									aQuantity,
								const Data::Item*							aItemData);
		bool				Destroy(
								uint32_t									aIndex,
								const Data::Item*							aItemData,
								uint32_t									aQuantity);
		bool				HasItems(
								uint32_t									aItemId,
								uint32_t									aQuantityRequired) const;
		void				RemoveItems(
								uint32_t									aItemId,
								uint32_t									aQuantity);
		bool				Consume(
								const Data::Ability::ConsumeItems*			aConsumeItems);
		bool				CanConsume(
								const Data::Ability::ConsumeItems*			aConsumeItems) const;
		bool				Move(
								uint32_t									aSourceIndex,
								const Data::Item*							aSourceItemData,
								uint32_t									aDestinationIndex,
								uint32_t									aSplitQuantity);
		const ItemInstance*	GetItemAtIndex(
								uint32_t									aIndex) const;
		ItemInstance*		GetItemAtIndex(
								uint32_t									aIndex);
		void				ForEach(
								std::function<void(const ItemInstance&)>	aCallback) const;
		size_t				GetSpace() const;

		// Public data
		std::vector<Entry>		m_entries;
		uint32_t				m_version = 0;
	};
}