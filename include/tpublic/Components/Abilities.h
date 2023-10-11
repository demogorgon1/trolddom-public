#pragma once

#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Abilities
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_ABILITIES;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

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

			// Public data
			std::vector<uint32_t>	m_available;
		};

	}

}