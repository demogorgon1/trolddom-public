#pragma once

#include "../ComponentBase.h"
#include "../MemoryReader.h"
#include "../MemoryWriter.h"
#include "../ObjectiveInstanceBase.h"

namespace tpublic
{

	namespace Components
	{

		struct ActiveQuests
			: public ComponentBase
		{
		public:
			static const Component::Id ID = Component::ID_ACTIVE_QUESTS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			struct Quest
			{
				void
				ToStream(
					IWriter*														aWriter) const
				{
					aWriter->WriteUInt(m_questId);
					aWriter->WriteUInt(m_objectiveInstanceData.size());
					aWriter->Write(&m_objectiveInstanceData[0], m_objectiveInstanceData.size());
				}

				bool
				FromStream(
					IReader*														aReader) 
				{
					if(!aReader->ReadUInt(m_questId))
						return false;

					size_t size;
					if(!aReader->ReadUInt(size))
						return false;

					if(size > 100000)
						return false;

					m_objectiveInstanceData.resize(size);
					if(aReader->Read(&m_objectiveInstanceData[0], m_objectiveInstanceData.size()) != m_objectiveInstanceData.size())
						return false;
					return true;
				}

				void
				SaveObjectiveInstanceData(
					const std::vector<std::shared_ptr<ObjectiveInstanceBase>>&		aObjectiveInstances)
				{
					m_objectiveInstanceData.clear();
					MemoryWriter writer(m_objectiveInstanceData);

					for(const std::shared_ptr<ObjectiveInstanceBase>& objectiveInstance : aObjectiveInstances)
						objectiveInstance->ToStream(&writer);
				}

				bool
				LoadObjectiveInstanceData(
					std::vector<std::shared_ptr<ObjectiveInstanceBase>>&			aObjectiveInstances)
				{
					MemoryReader reader(&m_objectiveInstanceData[0], m_objectiveInstanceData.size());

					for (std::shared_ptr<ObjectiveInstanceBase>& objectiveInstance : aObjectiveInstances)
					{
						if(!objectiveInstance->FromStream(&reader))
							return false;
					}

					if(!reader.IsEnd())
						return false;

					return true;
				}

				// Public data
				uint32_t											m_questId = 0;
				std::vector<uint8_t>								m_objectiveInstanceData;
			};

			enum Field
			{
				FIELD_QUESTS
			};

			static void
			CreateSchema(
				ComponentSchema*													aSchema)
			{
				aSchema->DefineCustomObjectPointersNoSource<Quest>(FIELD_QUESTS, offsetof(ActiveQuests, m_quests));
			}

			// Public data
			std::vector<std::unique_ptr<Quest>>						m_quests;
		};

	}

}