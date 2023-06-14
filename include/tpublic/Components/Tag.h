#pragma once

#include "../Component.h"
#include "../PlayerTag.h"

namespace tpublic
{

	namespace Components
	{

		struct Tag
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_TAG;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OTHERS;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			Tag()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~Tag()
			{

			}

			// ComponentBase implementation
			void				
			FromSource(
				const Parser::Node*		/*aSource*/) 
			{				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				m_playerTag.ToStream(aStream);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!m_playerTag.FromStream(aStream))
					return false;
				return true;
			}

			// Public data
			PlayerTag		m_playerTag;
		};
	}

}