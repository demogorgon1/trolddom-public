#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct VisibleAuras
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_VISIBLE_AURAS;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			enum AuraFlag : uint8_t
			{
				AURA_FLAG_STUNNED		= 0x01,
				AURA_FLAG_IMMOBILIZED	= 0x02,
			};

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
					aStream->WriteUInt(m_charges);
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
					if (!aStream->ReadUInt(m_charges))
						return false;
					return true;
				}

				// Public data
				uint32_t		m_auraId = 0;
				uint32_t		m_entityInstanceId = 0;
				int32_t			m_start = 0;
				int32_t			m_end = 0;
				uint32_t		m_charges = 0;
			};

			enum Field
			{
				FIELD_ENTRIES,
				FIELD_AURA_FLAGS
			};
			
			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Entry>(FIELD_ENTRIES, offsetof(VisibleAuras, m_entries));
				aSchema->DefineCustomPODNoSource<uint8_t>(FIELD_AURA_FLAGS, offsetof(VisibleAuras, m_auraFlags));
			}

			bool 
			HasAura(
				uint32_t			aAuraId) const
			{
				for(const Entry& t : m_entries)
				{
					if(t.m_auraId == aAuraId)
						return true;
				}
				return false;
			}

			void
			Reset()
			{
				m_entries.clear();
				m_auraFlags = 0;

				m_seq = 0;
			}

			// Helpers
			bool		IsStunned() const { return m_auraFlags & AURA_FLAG_STUNNED; }
			bool		IsImmobilized() const { return m_auraFlags & AURA_FLAG_IMMOBILIZED; }

			// Public data
			std::vector<Entry>	m_entries;			
			uint8_t				m_auraFlags = 0;

			// Internal
			uint32_t			m_seq = 0;
		};
	}

}