#pragma once

#include "../Component.h"
#include "../Stat.h"
#include "../Vec2.h"

namespace kpublic
{

	namespace Components
	{

		struct PlayerPublic
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_PUBLIC;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PLAYER_ONLY;

			PlayerPublic()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual
			~PlayerPublic()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_classId);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_classId))
					return false;
				return true;
			}

			// Public data
			uint32_t			m_classId = 0;
			Stat::Collection	m_stats;
		};
	}

}