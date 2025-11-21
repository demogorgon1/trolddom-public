#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct PVPRift
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PVP_RIFT;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_OPEN_TIME_STAMP
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT64, FIELD_OPEN_TIME_STAMP, NULL, offsetof(PVPRift, m_openTimeStamp));
			}

			void
			Reset()
			{
				m_openTimeStamp = 0;
			}

			// Public data
			uint64_t			m_openTimeStamp = 0;
		};

	}

}