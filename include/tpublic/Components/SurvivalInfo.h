#pragma once

#include "../ComponentBase.h"
#include "../Survival.h"

namespace tpublic
{

	namespace Components
	{

		struct SurvivalInfo
			: public ComponentBase
		{
		public:
			static const Component::Id ID = Component::ID_SURVIVAL_INFO;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			struct Boss
			{
				void
				ToStream(
					IWriter*		aWriter) const
				{
					aWriter->WriteUInt(m_entityId);
					m_position.ToStream(aWriter);
				}

				bool
				FromStream(
					IReader*		aReader)
				{
					if(!aReader->ReadUInt(m_entityId))
						return false;
					if(!m_position.FromStream(aReader))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_entityId = 0;
				Vec2				m_position;
			};

			enum Field
			{
				FIELD_BOSSES,
				FIELD_STATE
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Boss>(FIELD_BOSSES, offsetof(SurvivalInfo, m_bosses));
				aSchema->DefineCustomPODNoSource<Survival::State>(FIELD_STATE, offsetof(SurvivalInfo, m_state));
			}

			void
			Reset()
			{
				m_bosses.clear();
				m_state = Survival::STATE_NONE;
			}

			// Public data
			std::vector<Boss>		m_bosses;
			Survival::State			m_state = Survival::STATE_NONE;
		};

	}

}