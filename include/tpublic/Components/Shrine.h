#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Shrine
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_SHRINE;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_PANTHEON_ID,
				FIELD_DEITY_ID,
				FIELD_TOP_DEITY,
				FIELD_CAN_BE_DESECRATED
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_PANTHEON_ID, "pantheon", offsetof(Shrine, m_pantheonId))->SetDataType(DataType::ID_PANTHEON);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_DEITY_ID, "deity", offsetof(Shrine, m_deityId))->SetDataType(DataType::ID_DEITY);
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_TOP_DEITY, "top_deity", offsetof(Shrine, m_topDeity));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_CAN_BE_DESECRATED, "can_be_desecrated", offsetof(Shrine, m_canBeDesecrated));
			}

			void
			Reset()
			{
				m_pantheonId = 0;
				m_deityId = 0;
				m_topDeity = false;
				m_canBeDesecrated = false;

				m_desecratedName.clear();
				m_tappedName.clear();
				m_defaultName.clear();
			}

			// Public data
			uint32_t	m_pantheonId = 0;
			uint32_t	m_deityId = 0;
			bool		m_topDeity = false;
			bool		m_canBeDesecrated = true;

			// Internal
			std::string	m_desecratedName;
			std::string	m_tappedName;
			std::string	m_defaultName;
		};
	}

}