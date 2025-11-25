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
				FIELD_OPEN_TIME_STAMP,
				FIELD_AURA_ID,
				FIELD_PRIMARY_ITEM_ID,
				FIELD_SECONDARY_ITEM_ID,
				FIELD_FACTION_ID,
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT64, FIELD_OPEN_TIME_STAMP, NULL, offsetof(PVPRift, m_openTimeStamp));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_AURA_ID, "aura", offsetof(PVPRift, m_auraId))->SetDataType(DataType::ID_AURA);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_PRIMARY_ITEM_ID, "primary_item", offsetof(PVPRift, m_primaryItemId))->SetDataType(DataType::ID_ITEM);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_SECONDARY_ITEM_ID, "secondary_item", offsetof(PVPRift, m_secondaryItemId))->SetDataType(DataType::ID_ITEM);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_FACTION_ID, "faction", offsetof(PVPRift, m_factionId))->SetDataType(DataType::ID_FACTION);
			}

			void
			Reset()
			{
				m_openTimeStamp = 0;
				m_auraId = 0;
				m_primaryItemId = 0;
				m_secondaryItemId = 0;
				m_factionId = 0;

				m_activated = false;
			}

			// Public data
			uint64_t			m_openTimeStamp = 0;
			uint32_t			m_auraId = 0;
			uint32_t			m_primaryItemId = 0;
			uint32_t			m_secondaryItemId = 0;
			uint32_t			m_factionId = 0;

			// Internal, server only
			bool				m_activated = false;
		};

	}

}