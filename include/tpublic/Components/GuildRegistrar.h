#pragma once

#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct GuildRegistrar
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_GUILD_REGISTRAR;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum Field
			{
				FIELD_REGISTRATION_PRICE_TIERS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32_ARRAY, FIELD_REGISTRATION_PRICE_TIERS, "registration_price_tiers", offsetof(GuildRegistrar, m_registrationPriceTiers));
			}

			void
			Reset()
			{
				m_registrationPriceTiers.clear();
			}

			// Public data
			std::vector<uint32_t>		m_registrationPriceTiers;
		};

	}

}