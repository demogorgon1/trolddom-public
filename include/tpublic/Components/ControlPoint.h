#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct ControlPoint
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_CONTROL_POINT;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_CONTROL_POINT_STATE_IDS,
				FIELD_INDEX
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_CONTROL_POINT_STATE_IDS, "states", offsetof(ControlPoint, m_controlPointStateIds))->SetDataType(DataType::ID_CONTROL_POINT_STATE);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_INDEX, "index", offsetof(ControlPoint, m_index));
			}

			void
			Reset()
			{
				m_index = 0;
				m_controlPointStateIds.clear();
			}

			void
			SetControlPointStateId(
				uint32_t				aControlPointStateId)
			{
				for(size_t i = 0; i < m_controlPointStateIds.size(); i++)
				{
					if(m_controlPointStateIds[i] == aControlPointStateId)
					{
						m_index = (uint32_t)i;
						return;
					}
				}
			}

			// Public data
			uint32_t				m_index = 0;
			std::vector<uint32_t>	m_controlPointStateIds;
		};
	}

}