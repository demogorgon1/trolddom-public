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
				aStream->WritePOD(m_trading);
			}

			bool
			FromStream(
				IReader*								aStream) 
			{
				if (!aStream->ReadPOD(m_item))
					return false;
				if(!aStream->ReadPOD(m_trading))
					return false;
				return true;
			}

			// Public data
			ItemInstance				m_item;
			bool						m_trading = false;
		};

		void				ToStream(
								IWriter*									aWriter) const;
		bool				FromStream(
								IReader*									aReader);
		bool				AddToInventory(
								const ItemInstance&							aItemInstance,
								const Data::Item*							aItemData,
								uint32_t									aSize,
								bool										aAutoGrow = false);
		bool				CanAddMultipleToInventory(
								uint32_t									aItemId,
								uint32_t									aQuantity,
								const Data::Item*							aItemData,
								uint32_t									aSize) const;
		bool				AddMultipleToInventory(
								uint32_t									aItemId,
								uint32_t									aQuantity,
								const Data::Item*							aItemData,
								uint32_t									aSize);
		bool				Destroy(
								uint32_t									aIndex,
								const Data::Item*							aItemData,
								uint32_t									aQuantity,
								uint32_t									aSize);
		bool				HasItems(
								uint32_t									aItemId,
								uint32_t									aQuantityRequired) const;
		void				RemoveItems(
								uint32_t									aItemId,
								uint32_t									aQuantity,
								uint32_t									aSize);
		bool				Consume(
								const Data::Ability::Items*					aConsumeItems,
								uint32_t									aSize);
		bool				CanConsume(
								const Data::Ability::Items*					aConsumeItems) const;
		uint32_t			GetMaxConsumeCount(
								const Data::Ability::Items*					aConsumeItems) const;
		bool				Move(
								uint32_t									aSourceIndex,
								const Data::Item*							aSourceItemData,
								uint32_t									aDestinationIndex,
								uint32_t									aSplitQuantity,
								uint32_t									aSize);
		const ItemInstance*	GetItemAtIndex(
								uint32_t									aIndex) const;
		ItemInstance*		GetItemAtIndex(
								uint32_t									aIndex);
		void				ForEach(
								std::function<void(const ItemInstance&)>	aCallback) const;
		size_t				GetSpace() const;
		size_t				GetItemCount(
								uint32_t									aItemId) const;
		void				Resize(
								size_t										aTargetSize);
		void				Shrink(
								size_t										aTargetSize);
		void				Reset();

		// Public data
		std::vector<Entry>		m_entries;
		uint32_t				m_version = 0;
	};
}