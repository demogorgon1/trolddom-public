#pragma once

#include "../ComponentBase.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Components
	{

		struct Position
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_POSITION;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PUBLIC;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;

			Position()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~Position()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				m_position.ToStream(aStream);
				aStream->WritePOD(m_block);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if (!m_position.FromStream(aStream))
					return false;
				if(!aStream->ReadPOD(m_block))
					return false;
				return true;
			}

			// Public data
			Vec2	m_position;
			bool	m_block = false;
		};
	}

}