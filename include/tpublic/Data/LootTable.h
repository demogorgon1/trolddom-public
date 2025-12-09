#pragma once

#include "../DataBase.h"
#include "../Requirement.h"
#include "../UIntRange.h"

namespace tpublic
{

	namespace Data
	{

		struct LootTable
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_LOOT_TABLE;
			static const bool TAGGED = true;

			struct CashRange
			{			
				CashRange()
				{

				}

				CashRange(
					const SourceNode*	aSource)
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
				enum Elite : uint8_t
				{
					ELITE_DOES_NOT_MATTER,
					ELITE_MUST_BE,
					ELITE_MUST_NOT_BE
				};

				Possibility()
				{

				}

				Possibility(
					const SourceNode*	aSource)
				{
					if(aSource->m_annotation)
					{
						if(aSource->m_annotation->IsIdentifier("must_be_elite"))
							m_elite = ELITE_MUST_BE;
						else if (aSource->m_annotation->IsIdentifier("must_not_be_elite"))
							m_elite = ELITE_MUST_NOT_BE;
						else
							TP_VERIFY(false, aSource->m_debugInfo, "Invalid elite annotation.");
					}

					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "weight")
							m_weight = aChild->GetUInt32();
						else if (aChild->m_name == "quantity")
							m_quantity = UIntRange(aChild);
						else if(aChild->m_name == "loot_group")
							m_lootGroupId = aChild->GetId(DataType::ID_LOOT_GROUP);
						else if (aChild->m_name == "loot_cooldown")
							m_lootCooldownId = aChild->GetId(DataType::ID_LOOT_COOLDOWN);
						else if (aChild->m_name == "creature_types")
							aChild->GetIdArray(DataType::ID_CREATURE_TYPE, m_creatureTypes);
						else if(aChild->m_tag == "requirement")
							m_requirements.push_back(Requirement(aChild));
						else if(aChild->m_name == "use_special_loot_cooldown")
							m_useSpecialLootCooldown = aChild->GetBool();
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
					m_quantity.ToStream(aStream);
					aStream->WriteUInts(m_creatureTypes);
					aStream->WriteObjects(m_requirements);
					aStream->WriteUInt(m_lootCooldownId);
					aStream->WriteBool(m_useSpecialLootCooldown);
					aStream->WritePOD(m_elite);
				}

				bool
				FromStream(
					IReader* aStream) 
				{
					if (!aStream->ReadUInt(m_weight))
						return false;
					if (!aStream->ReadUInt(m_lootGroupId))
						return false;
					if (!m_quantity.FromStream(aStream))
						return false;
					if (!aStream->ReadUInts(m_creatureTypes))
						return false;
					if(!aStream->ReadObjects(m_requirements))
						return false;
					if (!aStream->ReadUInt(m_lootCooldownId))
						return false;
					if(!aStream->ReadBool(m_useSpecialLootCooldown))
						return false;
					if (!aStream->ReadPOD(m_elite))
						return false;
					return true;
				}

				bool
				HasCreatureType(
					uint32_t	aCreatureTypeId) const
				{
					if(m_creatureTypes.empty())
						return true; // Doesn't matter what creature type it is

					if(aCreatureTypeId == 0)
						return false; // This possibility is for specific creature types only

					for(uint32_t t : m_creatureTypes)
					{
						if(t == aCreatureTypeId)
							return true;
					}
					return false;
				}

				// Public data
				uint32_t						m_weight = 1;
				uint32_t						m_lootGroupId = 0;
				UIntRange						m_quantity = { 1, 1 };
				std::vector<uint32_t>			m_creatureTypes;
				std::vector<Requirement>		m_requirements;
				uint32_t						m_lootCooldownId = 0;
				bool							m_useSpecialLootCooldown = false;
				Elite							m_elite = ELITE_DOES_NOT_MATTER;
			};

			struct Slot
			{
				static const size_t MAX_POSSIBILTY_COUNT = 32;

				Slot()
				{

				}

				Slot(
					const SourceNode*	aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "possibility")
							m_possibilities.push_back(Possibility(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});

					TP_VERIFY(m_possibilities.size() <= MAX_POSSIBILTY_COUNT, aSource->m_debugInfo, "Maximum possibility count exceeded by %zu.", m_possibilities.size() - MAX_POSSIBILTY_COUNT);
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
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "cash")
						{
							m_cash = CashRange(aChild->GetArray());
						}
						else if (aChild->m_name == "cash_multiplier")
						{
							m_cashMultiplier = aChild->GetFloat();
						}
						else if (aChild->m_name == "slots")
						{
							aChild->GetArray()->ForEachChild([&](
								const SourceNode* aSlot)
								{
									m_slots.push_back(std::make_unique<Slot>(aSlot));
								});
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteOptionalObject(m_cash);
				aStream->WriteObjectPointers(m_slots);
				aStream->WriteFloat(m_cashMultiplier);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadOptionalObject(m_cash))
					return false;
				if(!aStream->ReadObjectPointers(m_slots))
					return false;
				if(!aStream->ReadFloat(m_cashMultiplier))
					return false;
				return true;
			}

			// Public data
			std::optional<CashRange>			m_cash;
			float								m_cashMultiplier = 1.0f;
			std::vector<std::unique_ptr<Slot>>	m_slots;
		};

	}

}