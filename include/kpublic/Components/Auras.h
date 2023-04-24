#pragma once

#include "../AuraEffectBase.h"
#include "../ComponentBase.h"

namespace kpublic
{

	namespace Components
	{

		struct Auras
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_AURAS;
			static const uint8_t FLAGS = 0;

			struct Entry
			{
				uint32_t										m_auraId = 0;
				uint32_t										m_entityInstanceId = 0;
				uint32_t										m_start = 0;
				std::vector<std::unique_ptr<AuraEffectBase>>	m_effects;
			};

			Auras()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~Auras()
			{

			}

			// ComponentBase implementation
			void	ToStream(
						IWriter*				/*aStream*/) const override { }
			bool	FromStream(
						IReader*				/*aStream*/) override { return true; }

			// Public data
			std::vector<std::unique_ptr<Entry>>					m_entries;
		};
	}

}