#pragma once

#include "ItemInstance.h"

namespace tpublic
{

	namespace Data
	{
		struct Item;
	}

	class EntityInstance;
	class LootGenerator;
	class SourceNode;
	
	class ItemProspect
	{
	public:
		struct Node
		{
			enum Type : uint8_t
			{
				TYPE_NONE,
				TYPE_ITEM_TYPE_FLAGS,
				TYPE_RARITY,
				TYPE_LOOT_TABLE
			};

			static constexpr bool 
			IsCompoundType(
				Type									aType)
			{
				switch(aType)
				{
				case TYPE_NONE:
				case TYPE_ITEM_TYPE_FLAGS:
				case TYPE_RARITY:
					return true;
				default:
					return false;
				}
			}

						Node();
						Node(
							Type						aType,
							const SourceNode*			aSource);
						~Node();

			void		ToStream(
							IWriter*					aWriter) const;
			bool		FromStream(
							IReader*					aReader);
			const Node*	ResolveItem(
							const Data::Item*			aItem) const;

			// Public data
			Type								m_type = TYPE_NONE;
			std::vector<uint32_t>				m_params;		
			std::vector<std::unique_ptr<Node>>	m_children;			
		};

						ItemProspect();
						ItemProspect(
							const SourceNode*			aSource);
						~ItemProspect();

		void			ToStream(
							IWriter*					aWriter) const;
		bool			FromStream(
							IReader*					aReader);
		void			Resolve(
							std::mt19937&				aRandom,
							const LootGenerator*		aLootGenerator,
							const EntityInstance*		aPlayerEntityInstance,
							const Data::Item*			aItem,
							std::vector<ItemInstance>&	aOut) const;
		bool			CanResolve(
							const Data::Item*			aItem) const;

		// Public data
		std::unique_ptr<Node>					m_root;
	};
}