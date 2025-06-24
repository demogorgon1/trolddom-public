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
					TYPE_THREAD_RUN_CONDITION_ENTITY_NOT_DEAD
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
						if(aSource->m_annotation)
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
				Type								m_type = INVALID_TYPE;
				std::optional<UIntRange>			m_range;
				std::vector<uint32_t>				m_ids;
				std::vector<std::unique_ptr<Node>>	m_children;
			};

			struct Thread
			{
				Thread()
				{

				}

				Thread(
					const SourceNode*	aSource)
				{
					m_root = std::make_unique<Node>(aSource, true);
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteObjectPointer(m_root);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadObjectPointer(m_root))
						return false;
					return true;
				}

				// Public data
				std::unique_ptr<Node>				m_root;
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
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Thread>>	m_threads;
			UIntRange								m_waveSpawnDistance;
			uint32_t								m_waveMaxDistance = 32;
			bool									m_resetWhenInactive = false;
		};

	}

}