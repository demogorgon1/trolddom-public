#pragma once

#include "../ComponentBase.h"

namespace kaos_public
{

	namespace Components
	{

		struct Abilities
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_ABILITIES;
			static const uint8_t FLAGS = FLAG_PRIVATE | FLAG_PLAYER_ONLY;

			struct Cooldown
			{
				void
				ToStream(
					IWriter* aStream) const 
				{
					aStream->WriteUInt(m_abilityId);
					aStream->WriteUInt(m_start);
					aStream->WriteUInt(m_end);
				}

				bool
				FromStream(
					IReader* aStream) 
				{
					if (!aStream->ReadUInt(m_abilityId))
						return false;
					if (!aStream->ReadUInt(m_start))
						return false;
					if (!aStream->ReadUInt(m_end))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_abilityId = 0;
				uint32_t			m_start = 0;
				uint32_t			m_end = 0;
			};

			Abilities()
				: ComponentBase(ID, FLAGS)
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
				aStream->WriteObjects(m_cooldowns);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInts(m_available))
					return false;
				if (!aStream->ReadObjects(m_cooldowns))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>	m_available;
			std::vector<Cooldown>	m_cooldowns;
		};

	}

}