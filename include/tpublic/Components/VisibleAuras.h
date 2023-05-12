#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct VisibleAuras
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_VISIBLE_AURAS;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			struct Entry
			{
				void
				ToStream(
					IWriter*		aStream) const 
				{	
					aStream->WriteUInt(m_auraId);
					aStream->WriteUInt(m_entityInstanceId);
					aStream->WriteUInt(m_start);
					aStream->WriteUInt(m_end);
				}

				bool
				FromStream(
					IReader*		aStream) 
				{
					if (!aStream->ReadUInt(m_auraId))
						return false;
					if (!aStream->ReadUInt(m_entityInstanceId))
						return false;
					if (!aStream->ReadUInt(m_start))
						return false;
					if (!aStream->ReadUInt(m_end))
						return false;
					return true;
				}

				// Public data
				uint32_t		m_auraId = 0;
				uint32_t		m_entityInstanceId = 0;
				uint32_t		m_start = 0;
				uint32_t		m_end = 0;
			};

			VisibleAuras()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~VisibleAuras()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter*		aStream) const override
			{
				aStream->WriteObjects(m_entries);
			}

			bool
			FromStream(
				IReader*		aStream) override
			{
				if(!aStream->ReadObjects(m_entries))
					return false;
				return true;
			}

			// Public data
			std::vector<Entry>	m_entries;			
		};
	}

}