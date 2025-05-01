#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct KillContribution
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_KILL_CONTRIBUTION;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			struct Set
			{
				void
				ToStream(
					IWriter*		aWriter) const
				{
					aWriter->WriteUIntSet(m_ids);
				}

				bool
				FromStream(
					IReader*		aReader)
				{
					if(!aReader->ReadUIntSet(m_ids))
						return false;
					return true;
				}

				// Public data
				std::unordered_set<uint32_t>	m_ids;
			};

			enum Field
			{
				FIELD_CHARACTER_IDS,
				FIELD_ENTITY_INSTANCE_IDS
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectNoSource<Set>(FIELD_CHARACTER_IDS, offsetof(KillContribution, m_characterIds));
				aSchema->DefineCustomObjectNoSource<Set>(FIELD_ENTITY_INSTANCE_IDS, offsetof(KillContribution, m_entityInstanceIds));
			}

			void
			Reset()
			{
				m_characterIds.m_ids.clear();
				m_entityInstanceIds.m_ids.clear();
			}

			// Public data			
			Set			m_characterIds;
			Set			m_entityInstanceIds;
		};
	}

}