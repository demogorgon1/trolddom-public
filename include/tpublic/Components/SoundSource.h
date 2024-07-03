#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct SoundSource
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_SOUND_SOURCE;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_TICK,
				FIELD_SOUND
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_TICK, NULL, offsetof(SoundSource, m_tick));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_SOUND, NULL, offsetof(SoundSource, m_soundId));
			}

			void
			Reset()
			{
				m_tick = 0;
				m_soundId = 0;
			}

			// Public data
			int32_t			m_tick = 0;
			uint32_t		m_soundId = 0;
		};
	}

}