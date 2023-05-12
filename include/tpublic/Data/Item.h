#pragma once

#include "../DataBase.h"
#include "../EquipmentSlot.h"
#include "../Rarity.h"
#include "../Stat.h"

namespace tpublic
{

	namespace Data
	{

		struct Item
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ITEM;

			enum PropertyType : uint8_t
			{
				PROPERTY_TYPE_WEAPON_COOLDOWN,
				PROPERTY_TYPE_REQUIRED_LEVEL,
				PROPERTY_TYPE_WEAPON_DAMAGE_MIN,
				PROPERTY_TYPE_WEAPON_DAMAGE_MAX,
				PROPERTY_TYPE_RARITY,

				NUM_PROPERTY_TYPES
			};

			struct Property
			{
				void 
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WritePOD(m_type);
					aStream->WriteUInt(m_value);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if (!aStream->ReadPOD(m_type))
						return false;
					if (!aStream->ReadUInt(m_value))
						return false;
					return true;
				}

				// Public data
				PropertyType		m_type = PropertyType(0);
				uint32_t			m_value = 0;
			};

			struct AddedStat
			{
				void 
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WritePOD(m_id);
					aStream->WriteUInt(m_min);
					aStream->WriteUInt(m_max);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if (!aStream->ReadPOD(m_id))
						return false;
					if (!aStream->ReadUInt(m_min))
						return false;
					if (!aStream->ReadUInt(m_max))
						return false;
					return true;
				}

				// Public data
				Stat::Id			m_id = Stat::Id(0);
				uint32_t			m_min = 0;
				uint32_t			m_max = 0;
			};

			struct Node
			{
				Node()
				{

				}

				Node(
					const Parser::Node*	aSource,
					bool				aWeighted)
				{
					aSource->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "weapon_cooldown")
							m_properties.push_back({ PROPERTY_TYPE_WEAPON_COOLDOWN, aChild->GetUInt32() });
						else if (aChild->m_name == "required_level")
							m_properties.push_back({ PROPERTY_TYPE_REQUIRED_LEVEL, aChild->GetUInt32() });
						else if (aChild->m_name == "weapon_damage_min")
							m_properties.push_back({ PROPERTY_TYPE_WEAPON_DAMAGE_MIN, aChild->GetUInt32() });
						else if (aChild->m_name == "weapon_damage_max")
							m_properties.push_back({ PROPERTY_TYPE_WEAPON_DAMAGE_MAX, aChild->GetUInt32() });
						else if (aChild->m_name == "rarity")
							m_properties.push_back({ PROPERTY_TYPE_RARITY, Rarity::StringToId(aChild->GetIdentifier()) });
						else if (aChild->m_name == "icon")
							m_iconSpriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
						else if (aChild->m_name == "name")
							m_name = aChild->GetString();
						else if (aChild->m_name == "suffix")
							m_suffix = aChild->GetString();
						else if(aChild->m_name == "weighted_child")
							m_weightedChildren.push_back(std::make_unique<Node>(aChild, true));
						else if (aChild->m_name == "random_child")
							m_randomChildren.push_back(std::make_unique<Node>(aChild, false));
						else if (aChild->m_name == "child_chance" && !aWeighted)
							m_chance = aChild->GetProbability();
						else if (aChild->m_name == "child_weight" && aWeighted)
							m_weight = aChild->GetUInt32();
						else if (aChild->m_tag == "stat" && aChild->m_type == Parser::Node::TYPE_ARRAY && aChild->m_children.size() == 2)
							m_addedStats.push_back({ Stat::StringToId(aChild->m_name.c_str()), aChild->m_children[0]->GetUInt32(), aChild->m_children[1]->GetUInt32() });
						else if (aChild->m_tag == "stat" && aChild->m_type == Parser::Node::TYPE_NUMBER)
							m_addedStats.push_back({ Stat::StringToId(aChild->m_name.c_str()), aChild->GetUInt32(), aChild->GetUInt32() });
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});

					for(std::unique_ptr<Node>& child : m_weightedChildren)
						m_totalChildWeight += child->m_weight;

					TP_VERIFY((m_totalChildWeight == 0 && m_weightedChildren.size() == 0) || (m_totalChildWeight > 0 && m_weightedChildren.size() > 0),
						aSource->m_debugInfo, "Invalid children.");
				}

				void 
				ToStream(
					IWriter*			aStream) const
				{
					aStream->WriteObjects(m_properties);
					aStream->WriteString(m_name);
					aStream->WriteString(m_suffix);
					aStream->WriteUInt(m_iconSpriteId);
					aStream->WriteUInt(m_chance);
					aStream->WriteUInt(m_weight);
					aStream->WriteObjectPointers(m_weightedChildren);
					aStream->WriteObjectPointers(m_randomChildren);
				}

				bool
				FromStream(
					IReader*			aStream)
				{
					if(!aStream->ReadObjects(m_properties))
						return false;
					if (!aStream->ReadString(m_name))
						return false;
					if (!aStream->ReadString(m_suffix))
						return false;
					if (!aStream->ReadUInt(m_iconSpriteId))
						return false;
					if (!aStream->ReadUInt(m_chance))
						return false;
					if (!aStream->ReadUInt(m_weight))
						return false;
					if (!aStream->ReadObjectPointers(m_weightedChildren))
						return false;
					if (!aStream->ReadObjectPointers(m_randomChildren))
						return false;

					for (std::unique_ptr<Node>& child : m_weightedChildren)
						m_totalChildWeight += child->m_weight;

					return true;
				}

				// Public data
				std::vector<Property>				m_properties;
				std::vector<AddedStat>				m_addedStats;
				std::string							m_name;
				std::string							m_suffix;
				uint32_t							m_iconSpriteId = 0;
				uint32_t							m_chance = 0;
				uint32_t							m_weight = 1;				
				uint32_t							m_totalChildWeight = 0;
				std::vector<std::unique_ptr<Node>>	m_weightedChildren;
				std::vector<std::unique_ptr<Node>>	m_randomChildren;
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
					if(aChild->m_name == "equipment_slots")
					{
						aChild->ForEachChild([&](
							const Parser::Node* aEquipmentSlot)
						{
							EquipmentSlot::Id id = EquipmentSlot::StringToId(aEquipmentSlot->GetIdentifier());
							TP_VERIFY(id != EquipmentSlot::INVALID_ID, aEquipmentSlot->m_debugInfo, "'%s' is not a valid equipment slot.", aEquipmentSlot->GetIdentifier());
							m_equipmentSlots.push_back(id);
						});	
					}
					else if(aChild->m_name == "root")
					{
						m_root = std::make_unique<Node>(aChild, false);
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
				aStream->WriteOptionalObjectPointer(m_root);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadOptionalObjectPointer(m_root))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>		m_equipmentSlots;
			std::unique_ptr<Node>		m_root;
		};

	}

}