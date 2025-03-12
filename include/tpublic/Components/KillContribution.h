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
					aWriter->WriteUIntSet(m_characterIds);
				}

				bool
				FromStream(
					IReader*		aReader)
				{
					if(!aReader->ReadUIntSet(m_characterIds))
						return false;
					return true;
				}

				// Public data
				std::unordered_set<uint32_t>	m_characterIds;
			};

			enum Field
			{
				FIELD_SET
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectNoSource<Set>(FIELD_SET, offsetof(KillContribution, m_set));
			}

			void
			Reset()
			{
				m_set.m_characterIds.clear();
			}

			// Public data			
			Set			m_set;
		};
	}

}