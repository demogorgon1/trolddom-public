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
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OTHERS;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

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

			GuildRegistrar()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~GuildRegistrar()
			{

			}

			// ComponentBase implementation
			void	
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInts(m_registrationPriceTiers);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInts(m_registrationPriceTiers))
					return false;
				return true;
			}

			void
			DebugPrint() const override
			{
				printf("guild_registrar: registration_price_tiers=");
				for(uint32_t t : m_registrationPriceTiers)
					printf("%u ", t);
				printf("\n");
			}

			// Public data
			std::vector<uint32_t>		m_registrationPriceTiers;
		};

	}

}