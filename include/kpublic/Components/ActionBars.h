#pragma once

#include "../ActionBar.h"
#include "../Component.h"

namespace kpublic
{

	namespace Components
	{

		struct ActionBars
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_ACTION_BARS;
			static const uint8_t FLAGS = FLAG_PRIVATE | FLAG_PLAYER_ONLY | FLAG_PERSISTENT;			

			ActionBars()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~ActionBars()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter*	aStream) const override
			{
				aStream->WriteObjectPointers(m_actionBars);
			}

			bool
			FromStream(
				IReader*	aStream) override
			{
				if(!aStream->ReadObjectPointers(m_actionBars))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<ActionBar>>		m_actionBars;
		};
	}

}