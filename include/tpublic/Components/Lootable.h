#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../ItemInstance.h"
#include "../PlayerTag.h"

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

			bool
			GetAvailableLootByIndex(
				size_t					aIndex,
				ItemInstance&			aOut) const
			{
				if(aIndex >= m_availableLoot.size())
					return false;
				aOut = m_availableLoot[aIndex];
				return true;
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
				aStream->WriteUInt(m_version);
				m_playerTag.ToStream(aStream);
				if (m_playerTag.IsSet())
				{					
					aStream->WriteInt(m_availableCash);
					aStream->WriteObjects(m_availableLoot);
				}
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_lootTableId))
					return false;
				if (!aStream->ReadUInt(m_version))
					return false;
				if(!m_playerTag.FromStream(aStream))
					return false;
				if (m_playerTag.IsSet())
				{
					if (!aStream->ReadInt(m_availableCash))
						return false;
					if (!aStream->ReadObjects(m_availableLoot))
						return false;
				}
				return true;
			}

			// Public data
			uint32_t					m_lootTableId = 0;

			PlayerTag					m_playerTag;
			int64_t						m_availableCash = 0;
			std::vector<ItemInstance>	m_availableLoot;
			uint32_t					m_version = 0;
		};
	}

}