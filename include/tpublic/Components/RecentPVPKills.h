#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct RecentPVPKills
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_RECENT_PVP_KILLS;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			static const uint64_t MAX_TIME = 10 * 60;

			struct Entry
			{
				static const size_t MAX_TIME_STAMPS = 4;

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_characterId);
					for(size_t i = 0; i < MAX_TIME_STAMPS; i++)
						aWriter->WriteUInt(m_timeStamps[i]);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_characterId))
						return false;
					for(size_t i = 0; i < MAX_TIME_STAMPS; i++)
					{
						if(!aReader->ReadUInt(m_timeStamps[i]))
							return false;
					}
					return true;
				}

				void
				AddTimeStamp(
					uint64_t			aTimeStamp)
				{
					uint64_t oldest = UINT64_MAX;
					size_t oldestIndex = SIZE_MAX;

					for(size_t i = 0; i < MAX_TIME_STAMPS; i++)
					{
						uint64_t& t = m_timeStamps[i];

						if(t == 0)
						{
							t = aTimeStamp;
							return;
						}

						if(t < oldest || oldestIndex == SIZE_MAX)
						{
							oldest = t;
							oldestIndex = i;
						}
					}

					assert(oldestIndex != SIZE_MAX);
				
					if(aTimeStamp > oldest)
						m_timeStamps[oldestIndex] = aTimeStamp;
				}

				void
				Update(
					uint64_t			aMinTimeStamp)
				{
					for (size_t i = 0; i < MAX_TIME_STAMPS; i++)
					{
						uint64_t& t = m_timeStamps[i];

						if(t < aMinTimeStamp)
							t = 0;
					}
				}

				size_t
				GetCount(
					uint64_t			aMinTimeStamp) const
				{
					size_t count = 0;
					for (size_t i = 0; i < MAX_TIME_STAMPS; i++)
					{
						if(m_timeStamps[i] >= aMinTimeStamp)
							count++;
					}
					return count;
				}

				// Public data				
				uint32_t		m_characterId = 0;
				uint64_t		m_timeStamps[MAX_TIME_STAMPS] = { 0 };
			};

			struct EntryAccount
			{
				static const size_t MAX_TIME_STAMPS = 4;

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_accountId);
					for(size_t i = 0; i < MAX_TIME_STAMPS; i++)
						aWriter->WriteUInt(m_timeStamps[i]);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_accountId))
						return false;
					for(size_t i = 0; i < MAX_TIME_STAMPS; i++)
					{
						if(!aReader->ReadUInt(m_timeStamps[i]))
							return false;
					}
					return true;
				}

				void
				AddTimeStamp(
					uint64_t			aTimeStamp)
				{
					uint64_t oldest = UINT64_MAX;
					size_t oldestIndex = SIZE_MAX;

					for(size_t i = 0; i < MAX_TIME_STAMPS; i++)
					{
						uint64_t& t = m_timeStamps[i];

						if(t == 0)
						{
							t = aTimeStamp;
							return;
						}

						if(t < oldest || oldestIndex == SIZE_MAX)
						{
							oldest = t;
							oldestIndex = i;
						}
					}

					assert(oldestIndex != SIZE_MAX);
				
					if(aTimeStamp > oldest)
						m_timeStamps[oldestIndex] = aTimeStamp;
				}

				void
				Update(
					uint64_t			aMinTimeStamp)
				{
					for (size_t i = 0; i < MAX_TIME_STAMPS; i++)
					{
						uint64_t& t = m_timeStamps[i];

						if(t < aMinTimeStamp)
							t = 0;
					}
				}

				size_t
				GetCount(
					uint64_t			aMinTimeStamp) const
				{
					size_t count = 0;
					for (size_t i = 0; i < MAX_TIME_STAMPS; i++)
					{
						if(m_timeStamps[i] >= aMinTimeStamp)
							count++;
					}
					return count;
				}

				// Public data				
				uint64_t		m_accountId = 0;
				uint64_t		m_timeStamps[MAX_TIME_STAMPS] = { 0 };
			};

			enum Field
			{
				FIELD_ENTRY,
				FIELD_ENTRY_ACCOUNT
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObjectsNoSource<Entry>(FIELD_ENTRY, offsetof(RecentPVPKills, m_entries));
				aSchema->DefineCustomObjectsNoSource<Entry>(FIELD_ENTRY_ACCOUNT, offsetof(RecentPVPKills, m_entriesAccount));
			}

			void
			Reset()
			{
				m_entries.clear();
				m_entriesAccount.clear();
			}

			void
			AddTimeStamp(
				uint64_t				aAccountId,
				uint64_t				aTimeStamp,
				uint64_t				aMinTimeStamp)
			{
				bool found = false;

				for(EntryAccount& t : m_entriesAccount)
				{
					t.Update(aMinTimeStamp);

					if(t.m_accountId == aAccountId)
					{
						t.AddTimeStamp(aTimeStamp);
						found = true;
					}
				}

				if(!found)
				{
					m_entriesAccount.resize(m_entriesAccount.size() + 1);
					EntryAccount& newEntry = m_entriesAccount[m_entriesAccount.size() - 1];
					newEntry.m_accountId = aAccountId;
					newEntry.m_timeStamps[0] = aTimeStamp;
				}

				// Also prune
				for(size_t i = 0; i < m_entriesAccount.size(); i++)
				{
					EntryAccount& t = m_entriesAccount[i];
					if(t.GetCount(aMinTimeStamp) == 0)
					{
						Helpers::RemoveCyclicFromVector(m_entriesAccount, i);
						i--;
					}
				}
			}

			size_t
			GetCount(
				uint64_t				aAccountId,
				uint64_t				aMinTimeStamp) const
			{
				for (const EntryAccount& t : m_entriesAccount)
				{
					if (t.m_accountId == aAccountId)
						return t.GetCount(aMinTimeStamp);
				}
				return 0;
			}

			// Public data
			std::vector<Entry>			m_entries;
			std::vector<EntryAccount>	m_entriesAccount;
		};
	}

}