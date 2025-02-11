#include "Pcheader.h"

#include <tpublic/Data/Item.h>
#include <tpublic/Data/LootTable.h>

#include <tpublic/ItemProspect.h>
#include <tpublic/ItemType.h>
#include <tpublic/LootGenerator.h>
#include <tpublic/Manifest.h>
#include <tpublic/Rarity.h>
#include <tpublic/SourceNode.h>

namespace tpublic
{

	ItemProspect::Node::Node()
	{

	}

	ItemProspect::Node::Node(
		Type							aType,
		const SourceNode*				aSource)
		: m_type(aType)
	{
		switch(m_type)
		{
		case TYPE_ITEM_TYPE_FLAGS:
			{
				TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing item type flags annotation.");
				uint16_t f = 0;
				aSource->m_annotation->GetArray()->ForEachChild([&](
					const SourceNode* aChild)
				{
					f |= ItemType::StringToFlag(aChild->GetIdentifier());
				});
				m_params.push_back((uint32_t)f);
			}
			break;

		case TYPE_RARITY:
			TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing rarity annotation.");
			aSource->m_annotation->GetArray()->ForEachChild([&](
				const SourceNode* aChild)
			{
				m_params.push_back((uint32_t)Rarity::StringToId(aChild->GetIdentifier()));
			});
			break;

		case TYPE_LOOT_TABLE:
			m_params.push_back(aSource->GetId(DataType::ID_LOOT_TABLE));
			break;

		default:
			break;
		}

		if(IsCompoundType(m_type))
		{
			aSource->GetObject()->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "item_type_flags")
					m_children.push_back(std::make_unique<Node>(TYPE_ITEM_TYPE_FLAGS, aChild));
				else if (aChild->m_name == "rarity")
					m_children.push_back(std::make_unique<Node>(TYPE_RARITY, aChild));
				else if (aChild->m_name == "loot_table")
					m_children.push_back(std::make_unique<Node>(TYPE_LOOT_TABLE, aChild));
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}
	}
	
	ItemProspect::Node::~Node()
	{
	
	}

	void		
	ItemProspect::Node::ToStream(
		IWriter*						aWriter) const
	{
		aWriter->WritePOD(m_type);
		aWriter->WriteUInts(m_params);

		if(IsCompoundType(m_type))
			aWriter->WriteObjectPointers(m_children);
	}
	
	bool		
	ItemProspect::Node::FromStream(
		IReader*						aReader)
	{
		if(!aReader->ReadPOD(m_type))
			return false;
		if(!aReader->ReadUInts(m_params))
			return false;

		if(IsCompoundType(m_type))
		{
			if (!aReader->ReadObjectPointers(m_children))
				return false;
		}
		return true;
	}
	
	const ItemProspect::Node*
	ItemProspect::Node::ResolveItem(
		const Data::Item*				aItem) const
	{		
		if(IsCompoundType(m_type))
			return this;

		bool match = false;

		switch(m_type)
		{
		case TYPE_ITEM_TYPE_FLAGS:
			{
				assert(m_params.size() == 1);
				const ItemType::Info* info = ItemType::GetInfo(aItem->m_itemType);
				uint32_t f = (uint32_t)info->m_flags;
				if ((f & m_params[0]) == m_params[0])
					match = true;
			}
			break;

		case TYPE_RARITY:
			for(uint32_t rarity : m_params)
			{
				if(aItem->m_rarity == (Rarity::Id)rarity)
				{
					match = true;
					break;
				}
			}
			break;

		default:
			match = true;
			break;
		}

		if(!match)
			return NULL;

		for(const std::unique_ptr<Node>& child : m_children)
		{
			const Node* node = child->ResolveItem(aItem);
			if(node != NULL)
				return node;
		}

		return NULL;
	}

	//-------------------------------------------------------------------------------------------

	ItemProspect::ItemProspect()
	{

	}

	ItemProspect::ItemProspect(
		const SourceNode*				aSourceNode)
	{
		m_root = std::make_unique<Node>(Node::TYPE_NONE, aSourceNode);
	}
	
	ItemProspect::~ItemProspect()
	{
	
	}

	void			
	ItemProspect::ToStream(
		IWriter*						aWriter) const
	{
		aWriter->WriteObjectPointer(m_root);
	}

	bool			
	ItemProspect::FromStream(
		IReader*						aReader)
	{
		if(!aReader->ReadObjectPointer(m_root))
			return false;
		return true;
	}
	
	void			
	ItemProspect::Resolve(
		std::mt19937&					aRandom,
		const LootGenerator*			aLootGenerator,
		const EntityInstance*			aPlayerEntityInstance,
		const Data::Item*				aItem,
		std::vector<ItemInstance>&		aOut) const
	{
		const Node* node = m_root->ResolveItem(aItem);
		if(node != NULL)
		{
			switch (node->m_type)
			{
			case Node::TYPE_LOOT_TABLE:
			{
				assert(node->m_params.size() == 1);
				const Data::LootTable* lootTable = aLootGenerator->GetManifest()->GetById<Data::LootTable>(node->m_params[0]);

				std::vector<const EntityInstance*> playerInstances = { aPlayerEntityInstance };
				aLootGenerator->GenerateItems(aRandom, playerInstances, NULL, aItem->m_itemLevel, 0, lootTable, [&](
					const ItemInstance& aItemInstance)
				{
					// FIXME: add to existing quantity if found
					aOut.push_back(aItemInstance);
				});
			}
			break;

			default:
				break;
			}
		}
	}

	bool			
	ItemProspect::CanResolve(
		const Data::Item*				aItem) const
	{
		return m_root->ResolveItem(aItem) != NULL;
	}

}