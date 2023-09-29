#pragma once

#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Trade
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_TRADE;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			Trade()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~Trade()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_otherCharacterId);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_otherCharacterId))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_otherCharacterId = 0;
		};
	}

}