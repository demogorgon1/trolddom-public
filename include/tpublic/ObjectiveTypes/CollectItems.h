#pragma once

#include "../ObjectiveTypeBase.h"

namespace tpublic::ObjectiveTypes
{

	class CollectItems
		: public ObjectiveTypeBase
	{
	public:
		static const ObjectiveType::Id ID = ObjectiveType::ID_COLLECT_ITEMS;
		static const uint32_t FLAGS = FLAG_WATCH_INVENTORY | FLAG_UNMANAGED;

								CollectItems();
		virtual					~CollectItems();

		// ObjectiveTypeBase implementation
		void					FromSource(
									const SourceNode*		aSource) override;
		void					ToStream(
									IWriter*				aWriter) override;
		bool					FromStream(
									IReader*				aReader) override;
		ObjectiveInstanceBase*	CreateInstance() const override;
		void					PostCompletionInventoryUpdate(
									Components::Inventory*	aInventory) const override;

	private:

		class Instance;

		uint32_t				m_itemId = 0;
		uint32_t				m_count = 1;
	};

}