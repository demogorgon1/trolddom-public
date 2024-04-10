#include "../Pcheader.h"

#include <tpublic/Components/Inventory.h>

#include <tpublic/ObjectiveTypes/CollectItems.h>

#include <tpublic/ObjectiveInstanceBase.h>
#include <tpublic/SourceNode.h>

namespace tpublic::ObjectiveTypes
{

	class CollectItems::Instance
		: public ObjectiveInstanceBase
	{
	public:
		Instance(
			const CollectItems*				aObjective)
			: m_objective(aObjective)
		{

		}

		virtual ~Instance()
		{

		}

		// ObjectiveInstanceBase implementation
		void				
		OnInventoryUpdate(
			const Components::Inventory*	aInventory) override
		{
			uint32_t collected = 0;

			for(const ItemList::Entry& entry : aInventory->m_itemList.m_entries)
			{
				if(entry.m_item.m_itemId == m_objective->m_itemId)
					collected += entry.m_item.m_quantity;
			}

			if(collected > m_objective->m_count)
				collected = m_objective->m_count;

			if(collected != m_collected)
			{
				m_collected = collected;

				OnUpdate();
			}
		}

		bool	
		GetProgress(
			Progress&						aOut) override
		{
			aOut = { m_collected, m_objective->m_count };
			return true;
		}

		bool	
		IsCompleted() const override
		{
			return m_collected == m_objective->m_count;
		}

		uint32_t
		GetHash() const override
		{
			return m_collected;
		}

		void	
		ToStream(
			IWriter*						aWriter) const override
		{
			aWriter->WriteUInt(m_collected);
		}
		
		bool	
		FromStream(
			IReader*						aReader) override
		{
			if(!aReader->ReadUInt(m_collected))
				return false;
			return true;
		}

	private:
		
		const CollectItems*	m_objective = NULL;
		uint32_t			m_collected = 0;
	};

	//-------------------------------------------------------------------------------------------

	CollectItems::CollectItems()
		: ObjectiveTypeBase(ID, FLAGS)
	{

	}
	
	CollectItems::~CollectItems()
	{

	}

	//-------------------------------------------------------------------------------------------

	void		
	CollectItems::FromSource(
		const SourceNode*					aSource)
	{
		aSource->ForEachChild([&](
			const SourceNode* aChild)
		{
			if(aChild->m_name == "item")
				m_itemId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ITEM, aChild->GetIdentifier());
			else if(aChild->m_name == "count")  
				m_count = aChild->GetUInt32();
			else
				TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());							
		});
	}

	void		
	CollectItems::ToStream(
		IWriter*							aWriter) 
	{
		aWriter->WriteUInt(m_itemId);
		aWriter->WriteUInt(m_count);
	}
	
	bool		
	CollectItems::FromStream(
		IReader*							aReader) 
	{
		if (!aReader->ReadUInt(m_itemId))
			return false;
		if (!aReader->ReadUInt(m_count))
			return false;
		return true;
	}

	ObjectiveInstanceBase* 
	CollectItems::CreateInstance() const 
	{
		return new Instance(this);
	}

	void					
	CollectItems::PostCompletionInventoryUpdate(
		Components::Inventory*				aInventory) const 
	{
		aInventory->m_itemList.RemoveItems(m_itemId, m_count);
		aInventory->SetPendingPersistenceUpdate(ComponentBase::PENDING_PERSISTENCE_UPDATE_MEDIUM_PRIORITY);
	}

}