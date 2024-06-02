#pragma once

#include "../DataBase.h"
#include "../Expr.h"
#include "../IntRange.h"

namespace tpublic
{

	namespace Data
	{

		struct Encounter
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ENCOUNTER;
			static const bool TAGGED = false;

			enum ConditionType : uint8_t
			{
				INVALID_CONDITION_TYPE,

				CONDITION_TYPE_ENTITY_HEALTH_LESS_THAN,
				CONDITION_TYPE_ENTITY_HEALTH_GREATER_THAN
			};

			static ConditionType
			SourceToConditionType(
				const SourceNode*		aSource)
			{
				std::string_view t(aSource->GetIdentifier());
				if (t == "entity_health_less_than")
					return CONDITION_TYPE_ENTITY_HEALTH_LESS_THAN;
				else if (t == "entity_health_greater_than")
					return CONDITION_TYPE_ENTITY_HEALTH_GREATER_THAN;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid condition type.", aSource->GetIdentifier());
				return INVALID_CONDITION_TYPE;
			}

			static DataType::Id
			ConditionTypeToDataType(
				ConditionType			aConditionType)
			{
				switch(aConditionType)
				{
				case CONDITION_TYPE_ENTITY_HEALTH_LESS_THAN:
				case CONDITION_TYPE_ENTITY_HEALTH_GREATER_THAN:
					return DataType::ID_ENTITY;
				default:	
					break;
				}
				return DataType::INVALID_ID;
			}

			struct Condition				
			{
				Condition()
				{

				}

				Condition(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing condition type annotation.");
					m_type = SourceToConditionType(aSource->m_annotation.get());

					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "id")
							m_id = aChild->m_sourceContext->m_persistentIdTable->GetId(ConditionTypeToDataType(m_type), aChild->GetIdentifier());
						else if(aChild->m_name == "value")
							m_value = aChild->GetUInt32();
						else 
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_type);
					aWriter->WriteUInt(m_id);
					aWriter->WriteUInt(m_value);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					if (!aReader->ReadUInt(m_id))
						return false;
					if (!aReader->ReadUInt(m_value))
						return false;
					return true;
				}

				// Public data
				ConditionType			m_type = INVALID_CONDITION_TYPE;
				uint32_t				m_id = 0;
				uint32_t				m_value = 0;
			};

			struct Spawn
			{
				Spawn()
				{

				}

				Spawn(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if (aChild->m_name == "map_entity_spawn")
							m_mapEntitySpawnId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_MAP_ENTITY_SPAWN, aChild->GetIdentifier());
						else if(aChild->m_name == "possible_entities")
							aChild->GetIdArray(DataType::ID_ENTITY, m_possibleEntityIds);
						else if (aChild->m_name == "entity_count")
							m_entityCount = UIntRange(aChild);
						else if (aChild->m_name == "interval")
							m_interval = IntRange(aChild);
						else if (aChild->m_name == "condition")
							m_conditions.push_back(Condition(aChild));
						else if(aChild->m_name == "delay_ticks")
							m_delayTicks = aChild->GetInt32();
						else if(aChild->m_name == "detached_from_spawn")
							m_detachedFromSpawn = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_mapEntitySpawnId);
					aWriter->WriteUInts(m_possibleEntityIds);
					m_entityCount.ToStream(aWriter);
					m_interval.ToStream(aWriter);
					aWriter->WriteInt(m_delayTicks);
					aWriter->WriteObjects(m_conditions);
					aWriter->WriteBool(m_detachedFromSpawn);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadUInt(m_mapEntitySpawnId))
						return false;
					if (!aReader->ReadUInts(m_possibleEntityIds))
						return false;
					if (!m_entityCount.FromStream(aReader))
						return false;
					if (!m_interval.FromStream(aReader))
						return false;
					if(!aReader->ReadInt(m_delayTicks))
						return false;
					if (!aReader->ReadObjects(m_conditions))
						return false;
					if(!aReader->ReadBool(m_detachedFromSpawn))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_mapEntitySpawnId = 0;
				std::vector<uint32_t>	m_possibleEntityIds;
				UIntRange				m_entityCount;
				IntRange				m_interval;
				int32_t					m_delayTicks = 0;
				std::vector<Condition>	m_conditions;
				bool					m_detachedFromSpawn = false;
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
						if(aChild->m_name == "spawn")
							m_spawns.push_back(std::make_unique<Spawn>(aChild));
						else if(aChild->m_name == "main_entities")
							aChild->GetIdArray(DataType::ID_ENTITY, m_mainEntityIds);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);

				aStream->WriteObjectPointers(m_spawns);
				aStream->WriteUInts(m_mainEntityIds);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;

				if(!aStream->ReadObjectPointers(m_spawns))
					return false;
				if(!aStream->ReadUInts(m_mainEntityIds))
					return false;
				return true;
			}

			// Public data			
			std::vector<std::unique_ptr<Spawn>>	m_spawns;
			std::vector<uint32_t>				m_mainEntityIds;
		};

	}

}