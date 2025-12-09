#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct DiminishingEffects
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_DIMINISHING_EFFECTS;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_NONE;

			struct Entry
			{
				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_diminishingEffectId);
					aWriter->WriteInt(m_resetTick);
					aWriter->WriteUInt(m_count);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if (!aReader->ReadUInt(m_diminishingEffectId))
						return false;
					if (!aReader->ReadInt(m_resetTick))
						return false;
					if (!aReader->ReadUInt(m_count))
						return false;
					return true;
				}			

				// Public data				
				uint32_t		m_diminishingEffectId = 0;
				int32_t			m_resetTick = 0;
				uint32_t		m_count = 0;
			};

			enum Field
			{
				FIELD_ENTRY
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Entry>(FIELD_ENTRY, offsetof(DiminishingEffects, m_entries));
			}

			void
			Reset()
			{
				m_entries.clear();
			}

			void
			Add(
				uint32_t				aDiminishingEffectId,
				int32_t					aResetTick)
			{
				for(Entry& t : m_entries)
				{
					if(t.m_diminishingEffectId == aDiminishingEffectId)
					{
						t.m_resetTick = aResetTick;
						t.m_count++;
						return;
					}
				}

				m_entries.resize(m_entries.size() + 1);
				Entry& newEntry = m_entries[m_entries.size() - 1];
				newEntry.m_diminishingEffectId = aDiminishingEffectId;
				newEntry.m_count = 1;
				newEntry.m_resetTick = aResetTick;
			}

			void
			Refresh(
				uint32_t				aDiminishingEffectId,
				int32_t					aResetTick)
			{
				for(Entry& t : m_entries)
				{
					if(t.m_diminishingEffectId == aDiminishingEffectId)
					{
						t.m_resetTick = aResetTick;
						return;
					}
				}
			}

			void
			Update(
				int32_t					aTick)
			{
				for(size_t i = 0; i < m_entries.size(); i++)
				{
					Entry& t = m_entries[i];

					if(aTick >= t.m_resetTick)
					{
						Helpers::RemoveCyclicFromVector(m_entries, i);
						i--;
					}
				}
			}

			uint32_t
			GetCount(
				uint32_t				aDiminishingEffectId) const
			{
				for (const Entry& t : m_entries)
				{
					if (t.m_diminishingEffectId == aDiminishingEffectId)
						return t.m_count;
				}
				return 0;
			}

			// Public data
			std::vector<Entry>	m_entries;
		};
	}

}