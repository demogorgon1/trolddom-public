#pragma once

#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Abilities
			: public ComponentBase
		{
		public:
			static const Component::Id ID = Component::ID_ABILITIES;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			enum Field : uint32_t
			{
				FIELD_AVAILABLE
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_AVAILABLE, NULL, offsetof(Abilities, m_available));
			}

			bool
			IsAbilityAvailable(
				uint32_t				aAbilityId) const
			{
				for(uint32_t abilityId : m_available)
				{
					if(abilityId == aAbilityId)
						return true;
				}
				return false;
			}

			void
			Reset()
			{
				m_available.clear();
			}

			std::vector<uint32_t>	m_available;
		};

	}

}