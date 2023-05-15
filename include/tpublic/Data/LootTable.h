#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct LootTable
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_LOOT_TABLE;

			struct CashRange
			{			
				CashRange()
				{

				}

				CashRange(
					const Parser::Node*	aSource)
				{
					TP_VERIFY(aSource->m_children.size() == 2, aSource->m_debugInfo, "Range must be an array with two elements.");
					m_min = (int64_t)aSource->m_children[0]->GetInt32();
					m_max = (int64_t)aSource->m_children[1]->GetInt32();
				}

				void
				ToStream(
					IWriter*			aStream) const 
				{
					aStream->WriteInt(m_min);
					aStream->WriteInt(m_max);
				}

				bool
				FromStream(
					IReader*			aStream) 
				{
					if (!aStream->ReadInt(m_min))
						return false;
					if (!aStream->ReadInt(m_max))
						return false;
					return true;
				}

				// Public data
				int64_t							m_min = 0;
				int64_t							m_max = 0;
			};

			struct Possibility
			{
				Possibility()
				{

				}

				Possibility(
					const Parser::Node*	aSource)
				{
					aSource->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "weight")
							m_weight = aChild->GetUInt32();
						else if(aChild->m_name == "loot_group")
							m_lootGroupId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_LOOT_GROUP, aChild->GetIdentifier());
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter* aStream) const 
				{
					aStream->WriteUInt(m_weight);
					aStream->WriteUInt(m_lootGroupId);
				}

				bool
				FromStream(
					IReader* aStream) 
				{
					if (!aStream->ReadUInt(m_weight))
						return false;
					if (!aStream->ReadUInt(m_lootGroupId))
						return false;
					return true;
				}

				// Public data
				uint32_t						m_weight = 1;
				uint32_t						m_lootGroupId = 0;
			};

			struct Slot
			{
				Slot()
				{

				}

				Slot(
					const Parser::Node*	aSource)
				{
					aSource->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "possibility")
							m_possibilities.push_back(Possibility(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter* aStream) const 
				{
					aStream->WriteObjects(m_possibilities);
				}

				bool
				FromStream(
					IReader* aStream) 
				{
					if(!aStream->ReadObjects(m_possibilities))
						return false;
					return true;
				}

				// Public data
				std::vector<Possibility>		m_possibilities;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node* aChild)
				{
					if(aChild->m_name == "cash")
					{
						m_cash = CashRange(aChild->GetArray());
					}
					else if (aChild->m_name == "slots")
					{
						aChild->GetArray()->ForEachChild([&](	
							const Parser::Node* aSlot)
						{
							m_slots.push_back(std::make_unique<Slot>(aSlot));
						});
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteOptionalObject(m_cash);
				aStream->WriteObjectPointers(m_slots);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadOptionalObject(m_cash))
					return false;
				if(!aStream->ReadObjectPointers(m_slots))
					return false;
				return true;
			}

			// Public data
			std::optional<CashRange>			m_cash;
			std::vector<std::unique_ptr<Slot>>	m_slots;
		};

	}

}