#pragma once

#include "../DataBase.h"
#include "../EntityState.h"

namespace tpublic
{

	namespace Data
	{

		struct MapEntitySpawn
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_ENTITY_SPAWN;

			struct SpawnCondition
			{
				enum Type : uint8_t
				{
					TYPE_NONE,
					TYPE_IF,
					TYPE_IF_NOT
				};

				struct SubCondition
				{
					SubCondition()
					{

					}

					SubCondition(
						const Parser::Node*	aNode)
					{
						if (aNode->m_name == "if")
							m_type = TYPE_IF;
						else if (aNode->m_name == "if_not")
							m_type = TYPE_IF_NOT;
						else
							TP_VERIFY(false, aNode->m_debugInfo, "'%s' is not a valid item.", aNode->m_name.c_str());

						m_mapTriggerId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_TRIGGER, aNode->GetIdentifier());
					}

					void
					ToStream(
						IWriter* aStream) const
					{
						aStream->WritePOD(m_type);
						aStream->WriteUInt(m_mapTriggerId);
					}

					bool
					FromStream(
						IReader* aStream)
					{
						if (!aStream->ReadPOD(m_type))
							return false;
						if (!aStream->ReadUInt(m_mapTriggerId))
							return false;
						return true;
					}

					// Public data
					Type										m_type = TYPE_NONE;
					uint32_t									m_mapTriggerId = 0;
				};

				SpawnCondition()
				{

				}

				SpawnCondition(
					const Parser::Node*	aNode)
				{
					aNode->GetObject()->ForEachChild([&](
						const Parser::Node* aChild)
					{
						m_subConditions.push_back(SubCondition(aChild));
					});
				}

				void
				ToStream(
					IWriter*		aStream) const
				{
					aStream->WriteObjects(m_subConditions);
				}

				bool
				FromStream(
					IReader*		aStream) 
				{
					if(!aStream->ReadObjects(m_subConditions))
						return false;
					return true;
				}

				// Public data
				std::vector<SubCondition>						m_subConditions;
			};

			struct Entity
			{
				void
				FromSource(
					const Parser::Node*	aNode)
				{
					m_entityId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ENTITY, aNode->m_name.c_str());

					aNode->GetObject()->ForEachChild([&](
						const Parser::Node* aChild)
					{
						if(aChild->m_name == "weight")
						{
							m_weight = aChild->GetUInt32();
						}
						else if (aChild->m_name == "init_state")
						{
							m_initState = EntityState::StringToId(aChild->GetIdentifier());
							TP_VERIFY(m_initState != EntityState::INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid entity state.", aChild->GetIdentifier());
						}
						else if(aChild->m_name == "spawn_condition")
						{
							m_spawnConditions.push_back(std::make_unique<SpawnCondition>(aChild));
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "Invalid 'entity' item.");
						}
					});
				}

				void
				ToStream(
					IWriter*		aStream) const
				{
					aStream->WriteUInt(m_entityId);
					aStream->WriteUInt(m_weight);
					aStream->WritePOD(m_initState);
					aStream->WriteObjectPointers(m_spawnConditions);
				}

				bool
				FromStream(
					IReader*		aStream) 
				{
					if (!aStream->ReadUInt(m_entityId))
						return false;
					if (!aStream->ReadUInt(m_weight))
						return false;
					if (!aStream->ReadPOD(m_initState))
						return false;
					if(!aStream->ReadObjectPointers(m_spawnConditions))
						return false;
					return true;
				}

				// Public data
				uint32_t										m_entityId = 0;
				uint32_t										m_weight = 1;
				EntityState::Id									m_initState = EntityState::ID_DEFAULT;
				std::vector<std::unique_ptr<SpawnCondition>>	m_spawnConditions;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aNode) override
			{
				aNode->GetObject()->ForEachChild([&](
					const Parser::Node* aChild)
				{
					if(aChild->m_tag == "entity")
					{
						std::unique_ptr<Entity> entity = std::make_unique<Entity>();
						entity->FromSource(aChild);
						m_entities.push_back(std::move(entity));
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "Invalid 'map_entity_spawn' item.");
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteObjectPointers(m_entities);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadObjectPointers(m_entities))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Entity>>									m_entities;
		};

	}

}