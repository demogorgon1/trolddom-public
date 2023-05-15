#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct Lootable
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_LOOTABLE;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OTHERS;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			Lootable()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~Lootable()
			{

			}

			// ComponentBase implementation
			void				
			FromSource(
				const Parser::Node*		aSource) 
			{
				aSource->ForEachChild([&](
					const Parser::Node* aChild)
				{
					if(aChild->m_name == "loot_table")
						m_lootTableId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_LOOT_TABLE, aChild->GetIdentifier());
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_lootTableId);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadUInt(m_lootTableId))
					return false;
				return true;
			}

			// Public data
			uint32_t	m_lootTableId = 0;
		};
	}

}