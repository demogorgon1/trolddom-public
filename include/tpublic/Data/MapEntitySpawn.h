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
			static const bool TAGGED = true;

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
						const SourceNode*	aNode)
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
					const SourceNode*	aNode)
				{
					aNode->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
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
					const SourceNode*	aNode)
				{
					m_entityId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ENTITY, aNode->m_name.c_str());

					aNode->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "level")
						{
							m_level = UIntRange(aChild);
						}
						else if(aChild->m_name == "weight")
						{
							m_weight = aChild->GetUInt32();
						}
						else if (aChild->m_name == "zones")
						{
							aChild->GetIdArray(DataType::ID_ZONE, m_zones);
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
					m_level.ToStream(aStream);
					aStream->WriteUInts(m_zones);
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
					if(!m_level.FromStream(aStream))
						return false;
					if(!aStream->ReadUInts(m_zones))
						return false;
					return true;
				}

				bool
				HasZone(
					uint32_t		aZoneId) const
				{
					if(m_zones.empty())
						return true;
					for(uint32_t t : m_zones)
					{
						if(t == aZoneId)
							return true;
					}
					return false;
				}

				bool
				HasLevel(
					uint32_t		aLevel) const
				{
					if(m_level.m_min == 0 && m_level.m_max == 0)
						return true;
					return aLevel >= m_level.m_min && aLevel <= m_level.m_max;
				}

				// Public data
				uint32_t										m_entityId = 0;
				uint32_t										m_weight = 1;
				EntityState::Id									m_initState = EntityState::ID_SPAWNING;
				std::vector<std::unique_ptr<SpawnCondition>>	m_spawnConditions;
				UIntRange										m_level;
				std::vector<uint32_t>							m_zones;
			};

			struct SpawnTimer
			{
				void
				FromSource(
					const SourceNode*	aNode)
				{
					aNode->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "min_delay")
							m_minDelay = aChild->GetInt32();
						else if (aChild->m_name == "cooldown_range_min")
							m_cooldownRangeMin = aChild->GetInt32();
						else if (aChild->m_name == "cooldown_range_max")
							m_cooldownRangeMax = aChild->GetInt32();
						else if (aChild->m_name == "despawn_required")
							m_despawnRequired = aChild->GetBool();
						else if (aChild->m_name == "only_when_triggered")
							m_onlyWhenTriggered = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*		aStream) const
				{
					aStream->WriteInt(m_minDelay);
					aStream->WriteInt(m_cooldownRangeMin);
					aStream->WriteInt(m_cooldownRangeMax);
					aStream->WriteBool(m_despawnRequired);
					aStream->WriteBool(m_onlyWhenTriggered);
				}

				bool
				FromStream(
					IReader*		aStream) 
				{
					if (!aStream->ReadInt(m_minDelay))
						return false;
					if (!aStream->ReadInt(m_cooldownRangeMin))
						return false;
					if (!aStream->ReadInt(m_cooldownRangeMax))
						return false;
					if (!aStream->ReadBool(m_despawnRequired))
						return false;
					if (!aStream->ReadBool(m_onlyWhenTriggered))
						return false;
					return true;
				}

				// Public data
				int32_t											m_minDelay = 10 * 10; // 10 secs
				int32_t											m_cooldownRangeMin = 30 * 10; // 30 secs
				int32_t											m_cooldownRangeMax = 50 * 10; // 50 secs
				bool											m_despawnRequired = false; 
				bool											m_onlyWhenTriggered = false;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{
				aNode->GetObject()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_tag == "entity")
						{
							std::unique_ptr<Entity> entity = std::make_unique<Entity>();
							entity->FromSource(aChild);
							m_entities.push_back(std::move(entity));
						}
						else if (aChild->m_name == "spawn_timer")
						{
							m_spawnTimer.FromSource(aChild);
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "Invalid 'map_entity_spawn' item.");
						}
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjectPointers(m_entities);
				m_spawnTimer.ToStream(aStream);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadObjectPointers(m_entities))
					return false;
				if(!m_spawnTimer.FromStream(aStream))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Entity>>									m_entities;
			int32_t																	m_minSpawnDelayTicks = 5 * 10; // 5 seconds
			SpawnTimer																m_spawnTimer;
		};

	}

}