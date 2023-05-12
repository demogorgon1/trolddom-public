#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct Wallet
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_WALLET;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;

			Wallet()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~Wallet()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_cash);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_cash))
					return false;
				return true;
			}

			// Public data
			uint64_t			m_cash = 0;
		};
	}

}