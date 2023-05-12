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

			Abilities()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~Abilities()
			{

			}

			// ComponentBase implementation
			void	
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInts(m_available);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInts(m_available))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>	m_available;
		};

	}

}