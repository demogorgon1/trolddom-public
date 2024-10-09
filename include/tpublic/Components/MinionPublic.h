#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct MinionPublic
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_MINION_PUBLIC;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_OWNER_ENTITY_INSTANCE,
				FIELD_CURRENT_MINION_MODE,
				FIELD_MINION_MODES
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_OWNER_ENTITY_INSTANCE, NULL, offsetof(MinionPublic, m_ownerEntityInstanceId));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CURRENT_MINION_MODE, "current_minion_mode", offsetof(MinionPublic, m_currentMinionModeId))->SetDataType(DataType::ID_MINION_MODE);
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_MINION_MODES, "minion_modes", offsetof(MinionPublic, m_minionModeIds))->SetDataType(DataType::ID_MINION_MODE);
			}

			void
			Reset()
			{
				m_ownerEntityInstanceId = 0;
				m_currentMinionModeId = 0;
				m_minionModeIds.clear();
			}

			bool
			HasMinionMode(
				uint32_t		aMinionModeId) const
			{
				for(uint32_t minionModeId : m_minionModeIds)
				{
					if(minionModeId == aMinionModeId)
						return true;
				}
				return false;
			}

			// Public data
			uint32_t				m_ownerEntityInstanceId = 0;
			uint32_t				m_currentMinionModeId = 0;
			std::vector<uint32_t>	m_minionModeIds;
		};
	}

}