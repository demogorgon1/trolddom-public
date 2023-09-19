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
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if (aChild->m_name == "registration_price_tiers")
						aChild->GetUIntArray(m_registrationPriceTiers);
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

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