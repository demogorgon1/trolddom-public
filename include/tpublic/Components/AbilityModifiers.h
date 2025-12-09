#pragma once

#include "../ComponentBase.h"
#include "../ToolTipMultiplier.h"

namespace tpublic
{

	namespace Components
	{

		struct AbilityModifiers
			: public ComponentBase
		{
		public:
			static const Component::Id ID = Component::ID_ABILITY_MODIFIERS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum Field : uint32_t
			{
				FIELD_ACTIVE,
				FIELD_TOOL_TIP_MULTIPLIERS,
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_ACTIVE, NULL, offsetof(AbilityModifiers, m_active));
				aSchema->DefineCustomObjectsNoSource<ToolTipMultiplier>(FIELD_TOOL_TIP_MULTIPLIERS, offsetof(AbilityModifiers, m_toolTipMultipliers));
			}

			bool
			HasActive(
				uint32_t			aAbilityModifierId) const
			{
				for(uint32_t t : m_active)
				{
					if(t == aAbilityModifierId)
						return true;
				}
				return false;
			}

			void
			Reset()
			{
				m_active.clear();
				m_toolTipMultipliers.clear();

				m_aurasSeq = 0;
			}

			// Public data
			std::vector<uint32_t>				m_active;
			std::vector<ToolTipMultiplier>		m_toolTipMultipliers;
			
			// Internal
			uint32_t							m_aurasSeq = 0;
		};

	}

}