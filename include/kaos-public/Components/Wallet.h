#pragma once

#include "Component.h"

namespace kaos_public
{

	namespace Components
	{

		struct Wallet
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_INVENTORY;
			static const uint8_t FLAGS = FLAG_SHARED_OWNER | FLAG_PLAYER_ONLY | FLAG_PERSISTENT;

			Wallet()
				: ComponentBase(ID, FLAGS)
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