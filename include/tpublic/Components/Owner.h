#pragma once

#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct Owner
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_OWNER;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PUBLIC;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			Owner()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~Owner()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_ownerEntityInstanceId);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_ownerEntityInstanceId))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_ownerEntityInstanceId = 0;
		};
	}

}