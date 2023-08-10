#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct Talents
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_TALENTS;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;

			struct Entry
			{
				void
				ToStream(
					IWriter*	aStream) const
				{
					aStream->WriteUInt(m_talentId);
					aStream->WriteUInt(m_points);
				}

				bool
				FromStream( 
					IReader*	aStream)
				{
					if(!aStream->ReadUInt(m_talentId))
						return false;
					if(!aStream->ReadUInt(m_points))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_talentId = 0;
				uint32_t				m_points = 0;
			};

			Talents()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~Talents()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter*		aStream) const override
			{
				aStream->WriteObjects(m_entries);
				aStream->WriteUInt(m_availablePoints);
			}

			bool
			FromStream(
				IReader*		aStream) override
			{
				if(!aStream->ReadObjects(m_entries))
					return false;
				if(!aStream->ReadUInt(m_availablePoints))
					return false;
				return true;
			}

			// Public data
			std::vector<Entry>			m_entries;
			uint32_t					m_availablePoints = 0;
		};
	}

}