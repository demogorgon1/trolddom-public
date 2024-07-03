#pragma once

#include "../ComponentBase.h"

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
				FIELD_ACTIVE
			};

			static void
			CreateSchema(
				ComponentSchema*	aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_ACTIVE, NULL, offsetof(AbilityModifiers, m_active));
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
			}

			// Public data
			std::vector<uint32_t>	m_active;
		};

	}

}