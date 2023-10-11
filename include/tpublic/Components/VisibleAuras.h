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
					aStream->WriteInt(m_start);
					aStream->WriteInt(m_end);
				}

				bool
				FromStream(
					IReader*		aStream) 
				{
					if (!aStream->ReadUInt(m_auraId))
						return false;
					if (!aStream->ReadUInt(m_entityInstanceId))
						return false;
					if (!aStream->ReadInt(m_start))
						return false;
					if (!aStream->ReadInt(m_end))
						return false;
					return true;
				}

				// Public data
				uint32_t		m_auraId = 0;
				uint32_t		m_entityInstanceId = 0;
				int32_t			m_start = 0;
				int32_t			m_end = 0;
			};

			enum Field
			{
				FIELD_ENTRIES
			};
			
			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Entry>(FIELD_ENTRIES, offsetof(VisibleAuras, m_entries));
			}

			// Public data
			std::vector<Entry>	m_entries;			
		};
	}

}