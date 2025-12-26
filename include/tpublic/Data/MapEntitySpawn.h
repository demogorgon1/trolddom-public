#pragma once

#include "../DataBase.h"
#include "../EntityState.h"
#include "../IntRange.h"
#include "../SpriteInfo.h"

namespace tpublic
{

	namespace Data
	{

		struct MapEntitySpawn
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_ENTITY_SPAWN;
			static const bool TAGGED = true;

			struct ControlPointRequirement
			{
				ControlPointRequirement()
				{

				}

				ControlPointRequirement(
					const SourceNode*		aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "entities")
							aChild->GetIdArray(DataType::ID_ENTITY, m_entityIds);
						else if(aChild->m_name == "control_point_state")
							m_controlPointStateId = aChild->GetId(DataType::ID_CONTROL_POINT_STATE);
						else if(aChild->m_name == "max_distance")
							m_maxDistance = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*				aStream) const
				{
					aStream->WriteUInts(m_entityIds);
					aStream->WriteUInt(m_controlPointStateId);
					aStream->WriteInt(m_maxDistance);
				}

				bool
				FromStream(
					IReader*				aStream)
				{
					if (!aStream->ReadUInts(m_entityIds))
						return false;
					if (!aStream->ReadUInt(m_controlPointStateId))
						return false;
					if (!aStream->ReadInt(m_maxDistance))
						return false;
					return true;
				}

				// Public data
				std::vector<uint32_t>	m_entityIds;
				uint32_t				m_controlPointStateId = 0;
				int32_t					m_maxDistance = 0;
			};

			struct SpawnCondition
			{
				enum Type : uint8_t
				{
					TYPE_NONE,
					TYPE_IF,
					TYPE_IF_NOT,
					TYPE_ENCOUNTER_NOT_ACTIVE,
					TYPE_REALM_BALANCE_ABOVE,
					TYPE_NO_NEARBY_ENTITY,
					TYPE_SEASONAL_EVENT_ACTIVE
				};

				struct SubCondition
				{
					SubCondition()
					{

					}

					SubCondition(
						const SourceNode*	aNode)
					{
						bool hasValue = false;

						if (aNode->m_name == "if")
						{
							m_type = TYPE_IF;
							m_dataType = DataType::ID_MAP_TRIGGER;
						}
						else if (aNode->m_name == "if_not")
						{
							m_type = TYPE_IF_NOT;
							m_dataType = DataType::ID_MAP_TRIGGER;
						}
						else if (aNode->m_name == "encounter_not_active")
						{
							m_type = TYPE_ENCOUNTER_NOT_ACTIVE;
							m_dataType = DataType::ID_ENCOUNTER;
						}
						else if (aNode->m_name == "realm_balance_above")
						{
							m_type = TYPE_REALM_BALANCE_ABOVE;
							m_dataType = DataType::ID_REALM_BALANCE;
							hasValue = true;
						}
						else if(aNode->m_name == "no_nearby_entity")
						{
							m_type = TYPE_NO_NEARBY_ENTITY;
							m_dataType = DataType::ID_ENTITY;
							hasValue = true;
						}
						else if (aNode->m_name == "seasonal_event_active")
						{
							m_type = TYPE_SEASONAL_EVENT_ACTIVE;
							m_dataType = DataType::ID_SEASONAL_EVENT;
						}
						else
						{
							TP_VERIFY(false, aNode->m_debugInfo, "'%s' is not a valid item.", aNode->m_name.c_str());
						}

						m_id = aNode->GetId(m_dataType);

						if(hasValue)
						{
							TP_VERIFY(aNode->m_annotation, aNode->m_debugInfo, "Missing annotation value.");
							m_value = aNode->m_annotation->GetInt32();
						}
					}

					void
					ToStream(
						IWriter* aStream) const
					{
						aStream->WritePOD(m_type);
						aStream->WriteUInt(m_id);
						aStream->WritePOD(m_dataType);
						aStream->WriteInt(m_value);
					}

					bool
					FromStream(
						IReader* aStream)
					{
						if (!aStream->ReadPOD(m_type))
							return false;
						if (!aStream->ReadUInt(m_id))
							return false;
						if (!aStream->ReadPOD(m_dataType))
							return false;
						if(!aStream->ReadInt(m_value))
							return false;
						return true;
					}

					// Public data
					Type										m_type = TYPE_NONE;
					DataType::Id								m_dataType = DataType::INVALID_ID;
					uint32_t									m_id = 0;
					int32_t										m_value = 0;
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
					m_entityId = aNode->m_sourceContext->m_persistentIdTable->GetId(aNode->m_debugInfo, DataType::ID_ENTITY, aNode->m_name.c_str());

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
						else if (aChild->m_name == "must_have_tile_flags")
						{
							m_mustHaveTileFlags = SpriteInfo::SourceToFlags(aChild);
						}
						else if (aChild->m_name == "must_not_have_tile_flags")
						{
							m_mustNotHaveTileFlags = SpriteInfo::SourceToFlags(aChild);
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
						else if(aChild->m_name == "control_point_requirement")
						{
							m_controlPointRequirement = ControlPointRequirement(aChild);
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
					aStream->WritePOD(m_mustHaveTileFlags);
					aStream->WritePOD(m_mustNotHaveTileFlags);
					aStream->WriteOptionalObject(m_controlPointRequirement);
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
					if(!aStream->ReadPOD(m_mustHaveTileFlags))
						return false;
					if (!aStream->ReadPOD(m_mustNotHaveTileFlags))
						return false;
					if(!aStream->ReadOptionalObject(m_controlPointRequirement))
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

				bool
				CheckTileFlags(
					uint16_t		aTileFlags) const
				{
					if((aTileFlags & m_mustNotHaveTileFlags) != 0)
						return false;
					return (aTileFlags & m_mustHaveTileFlags) == m_mustHaveTileFlags;
				}

				// Public data
				uint32_t										m_entityId = 0;
				uint32_t										m_weight = 1;
				EntityState::Id									m_initState = EntityState::ID_SPAWNING;
				std::vector<std::unique_ptr<SpawnCondition>>	m_spawnConditions;
				UIntRange										m_level;
				std::vector<uint32_t>							m_zones;
				uint16_t										m_mustHaveTileFlags = 0;
				uint16_t										m_mustNotHaveTileFlags = 0;
				std::optional<ControlPointRequirement>			m_controlPointRequirement;
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
						if(aChild->m_name == "normal")
							m_normal = IntRange(aChild);
						else if (aChild->m_name == "fast")
							m_fast = IntRange(aChild);
						else if (aChild->m_name == "fast_trigger")
							m_fastTrigger = aChild->GetInt32();
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
					m_normal.ToStream(aStream);
					m_fast.ToStream(aStream);
					aStream->WriteInt(m_fastTrigger);
					aStream->WriteBool(m_despawnRequired);
					aStream->WriteBool(m_onlyWhenTriggered);
				}

				bool
				FromStream(
					IReader*		aStream) 
				{
					if (!m_normal.FromStream(aStream))
						return false;
					if (!m_fast.FromStream(aStream))
						return false;
					if (!aStream->ReadInt(m_fastTrigger))
						return false;
					if (!aStream->ReadBool(m_despawnRequired))
						return false;
					if (!aStream->ReadBool(m_onlyWhenTriggered))
						return false;
					return true;
				}

				// Public data
				IntRange										m_normal = { 4 * 60 * 10, 5 * 60 * 10 }; // 4-5 minutes
				IntRange										m_fast = { 1 * 60 * 10, 2 * 60 * 10 }; // 1-2 minutes
				int32_t											m_fastTrigger = 2 * 60 * 10; // 2 minutes
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
						else if(aChild->m_name == "debug_color")
						{
							m_debugColor = Image::RGBA(aChild);
						}
						else if (aChild->m_name == "point_of_interest")
						{
							m_pointOfInterest = aChild->GetBool();
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
				aStream->WriteOptionalPOD(m_debugColor);
				aStream->WriteBool(m_pointOfInterest);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadObjectPointers(m_entities))
					return false;
				if(!m_spawnTimer.FromStream(aStream))
					return false;
				if(!aStream->ReadOptionalPOD(m_debugColor))
					return false;
				if(!aStream->ReadBool(m_pointOfInterest))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Entity>>									m_entities;
			int32_t																	m_minSpawnDelayTicks = 5 * 10; // 5 seconds
			SpawnTimer																m_spawnTimer;
			std::optional<Image::RGBA>												m_debugColor;
			bool																	m_pointOfInterest = false;
		};

	}

}