#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct SurvivalScript
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_SURVIVAL_SCRIPT;
			static const bool TAGGED = false;

			struct Node
			{
				enum Type : uint8_t
				{
					INVALID_TYPE,

					TYPE_WAIT,
					TYPE_REPEAT,
					TYPE_WAVE,
					TYPE_THREAD_RUN_CONDITION_ENTITY_NOT_DEAD,
					TYPE_SPAWN_BOSS,
					TYPE_THREAD_BOSS_MAP_ENTITY_SPAWNS
				};			

				Node()
				{

				}

				Node(
					const SourceNode*	aSource,
					bool				aRoot = false)
				{
					if(aRoot)
					{
						m_type = TYPE_REPEAT;
						m_range = { 1, 1 };
					}
					else
					{
						if(aSource->m_annotation && UIntRange::ValidateSourceNode(aSource->m_annotation.get()))
							m_range = UIntRange(aSource->m_annotation.get());

						if (aSource->m_name == "wait")
						{
							m_type = TYPE_WAIT;
						}
						else if (aSource->m_name == "repeat")
						{
							m_type = TYPE_REPEAT;							
						}
						else if (aSource->m_name == "wave")
						{
							m_type = TYPE_WAVE;
							aSource->GetIdArray(DataType::ID_ENTITY, m_ids);
						}
						else if (aSource->m_name == "thread_run_condition_entity_not_dead")
						{
							m_type = TYPE_THREAD_RUN_CONDITION_ENTITY_NOT_DEAD;
							aSource->GetIdArray(DataType::ID_ENTITY, m_ids);
						}
						else if (aSource->m_name == "spawn_boss")
						{
							m_type = TYPE_SPAWN_BOSS;
							aSource->GetIdArray(DataType::ID_ENTITY, m_ids);
						}
						else if (aSource->m_name == "thread_boss_map_entity_spawns")
						{
							m_type = TYPE_THREAD_BOSS_MAP_ENTITY_SPAWNS;
							aSource->GetIdArray(DataType::ID_MAP_ENTITY_SPAWN, m_ids);
						}
						else
						{
							TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid item.", aSource->m_name.c_str());
						}
					}

					if(m_type == TYPE_REPEAT)
					{
						aSource->GetObject()->ForEachChild([&](
							const SourceNode* aChild)
						{
							m_children.push_back(std::make_unique<Node>(aChild));
						});
					}						
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_type);
					aWriter->WriteOptionalObject(m_range);
					aWriter->WriteUInts(m_ids);
					aWriter->WriteObjectPointers(m_children);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					if (!aReader->ReadOptionalObject(m_range))
						return false;
					if (!aReader->ReadUInts(m_ids))
						return false;
					if (!aReader->ReadObjectPointers(m_children))
						return false;
					return true;
				}

				// Public data
				Type									m_type = INVALID_TYPE;
				std::optional<UIntRange>				m_range;
				std::vector<uint32_t>					m_ids;
				std::vector<std::unique_ptr<Node>>		m_children;
			};

			struct Thread
			{
				Thread()
				{

				}

				Thread(
					const SourceNode*	aSource)
				{
					if(aSource->m_annotation)
						m_name = aSource->m_annotation->GetIdentifier();

					m_root = std::make_unique<Node>(aSource, true);
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteObjectPointer(m_root);
					aWriter->WriteString(m_name);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadObjectPointer(m_root))
						return false;
					if(!aReader->ReadString(m_name))
						return false;
					return true;
				}

				// Public data
				std::unique_ptr<Node>					m_root;
				std::string								m_name;
			};

			enum Condition : uint8_t
			{
				CONDITION_NONE,
				CONDITION_PARTY
			};

			struct ConditionalEntityMapEntry
			{
				ConditionalEntityMapEntry()
				{

				}

				ConditionalEntityMapEntry(
					const SourceNode*	aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing target entity annotation.");

					if(aSource->m_annotation->m_type == SourceNode::TYPE_OBJECT && aSource->m_annotation->m_children.size() == 0)
						m_entityId = 0; // Empty object means mapping to nothing
					else
						m_entityId = aSource->m_annotation->GetId(DataType::ID_ENTITY);

					if(aSource->IsIdentifier("party"))
						m_condition = CONDITION_PARTY;
					else
						TP_VERIFY(false, aSource->m_debugInfo, "Invalid condition.");
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_condition);
					aWriter->WriteUInt(m_entityId);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadPOD(m_condition))
						return false;
					if (!aReader->ReadUInt(m_entityId))
						return false;
					return true;
				}

				// Public data
				Condition								m_condition = CONDITION_NONE;
				uint32_t								m_entityId = 0;
			};

			struct ConditionalEntityMap
			{
				ConditionalEntityMap()
				{

				}

				ConditionalEntityMap(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						uint32_t entityId = aChild->m_sourceContext->m_persistentIdTable->GetId(aChild->m_debugInfo, DataType::ID_ENTITY, aChild->m_name.c_str());

						Table::iterator i = m_table.find(entityId);
						if(i == m_table.end())
							m_table[entityId] = { ConditionalEntityMapEntry(aChild) };
						else
							i->second.push_back(ConditionalEntityMapEntry(aChild));						
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_table.size());
					for(Table::const_iterator i = m_table.cbegin(); i != m_table.cend(); i++)
					{
						aWriter->WriteUInt(i->first);
						aWriter->WriteObjects(i->second);
					}
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					size_t count;
					if(!aReader->ReadUInt(count))
						return false;

					for(size_t i = 0; i < count; i++)
					{
						uint32_t entityId;
						if(!aReader->ReadUInt(entityId))
							return false;

						std::vector<ConditionalEntityMapEntry>& t = m_table[entityId];
						if(!aReader->ReadObjects(t))
							return false;
					}
					return true;
				}

				// Public data
				typedef std::unordered_map<uint32_t, std::vector<ConditionalEntityMapEntry>> Table;
				Table									m_table;
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
						if (aChild->m_name == "thread")
							m_threads.push_back(std::make_unique<Thread>(aChild));
						else if(aChild->m_name == "wave_spawn_distance")
							m_waveSpawnDistance = UIntRange(aChild);
						else if (aChild->m_name == "wave_max_distance")
							m_waveMaxDistance = aChild->GetUInt32();
						else if(aChild->m_name == "reset_when_inactive")
							m_resetWhenInactive = aChild->GetBool();
						else if(aChild->m_name == "conditional_entity_map")
							m_conditionalEntityMap = std::make_unique<ConditionalEntityMap>(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjectPointers(m_threads);
				m_waveSpawnDistance.ToStream(aStream);
				aStream->WriteUInt(m_waveMaxDistance);
				aStream->WriteBool(m_resetWhenInactive);
				aStream->WriteOptionalObjectPointer(m_conditionalEntityMap);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadObjectPointers(m_threads))
					return false;
				if(!m_waveSpawnDistance.FromStream(aStream))
					return false;
				if(!aStream->ReadUInt(m_waveMaxDistance))
					return false;
				if(!aStream->ReadBool(m_resetWhenInactive))
					return false;
				if(!aStream->ReadOptionalObjectPointer(m_conditionalEntityMap))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Thread>>		m_threads;
			UIntRange									m_waveSpawnDistance;
			uint32_t									m_waveMaxDistance = 32;
			bool										m_resetWhenInactive = false;
			std::unique_ptr<ConditionalEntityMap>		m_conditionalEntityMap;
		};

	}

}