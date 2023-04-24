#include "Pcheader.h"

#include <kpublic/ThreatTable.h>

namespace kpublic
{

	ThreatTable::ThreatTable()
		: m_head(NULL)
		, m_tail(NULL)
	{

	}
	
	ThreatTable::~ThreatTable()
	{
		Clear();
	}

	void	
	ThreatTable::Update(
		uint32_t					aTick)
	{
		Entry* t = m_head;
		while(t != NULL)
		{
			Entry* next = t->m_next;

			uint32_t ticksSinceLastUpdate = aTick - t->m_tick;
			if(ticksSinceLastUpdate > TIMEOUT_TICKS)
			{
				m_table.erase(t->m_entityInstanceId);
				_Remove(t);
			}

			t = next;
		}
	}
	
	void	
	ThreatTable::Add(
		uint32_t					aTick,
		uint32_t					aEntityInstanceId,
		int32_t						aThreat)
	{
		Entry* entry;

		Table::iterator i = m_table.find(aEntityInstanceId);
		if(i != m_table.end())
		{
			entry = i->second;

			entry->m_threat += aThreat;

			if(aThreat > 0)
			{
				// Move up the list	
				while(entry->m_prev != NULL && entry->m_prev->m_threat < entry->m_threat)
				{
					Entry* prev = entry->m_prev;

					if (entry->m_next != NULL)
						entry->m_next->m_prev = entry->m_prev;
					else
						m_tail = entry->m_prev;

					if (entry->m_prev != NULL)
						entry->m_prev->m_next = entry->m_next;
					else
						m_head = entry->m_next;

					entry->m_next = NULL;
					entry->m_prev = NULL;

					_InsertBefore(entry, prev);
				}
			}
			else
			{
				// Move down the list				
				while (entry->m_next != NULL && entry->m_next->m_threat > entry->m_threat)
				{
					Entry* next = entry->m_next;

					if (next->m_next != NULL)
						next->m_next->m_prev = next->m_prev;
					else
						m_tail = next->m_prev;

					if (next->m_prev != NULL)
						next->m_prev->m_next = next->m_next;
					else
						m_head = next->m_next;

					next->m_next = NULL;
					next->m_prev = NULL;

					_InsertBefore(next, entry);
				}
			}
		}
		else
		{
			entry = new Entry();
			m_table[aEntityInstanceId] = entry;

			entry->m_entityInstanceId = aEntityInstanceId;

			entry->m_threat = aThreat;

			Entry* insertBefore = _FindHighestLessThan(aThreat);
			if(insertBefore != NULL)
			{
				// Insert before this
				_InsertBefore(entry, insertBefore);
			}
			else
			{
				// Insert at end of list
				_InsertAtEnd(entry);
			}
		}

		entry->m_tick = aTick;
	}

	void	
	ThreatTable::Remove(
		uint32_t					aEntityInstanceId)
	{
		Table::iterator i = m_table.find(aEntityInstanceId);
		if (i != m_table.end())
		{
			_Remove(i->second);

			m_table.erase(i);
		}
	}

	void	
	ThreatTable::Clear()
	{
		while(m_head != NULL)
		{
			Entry* next = m_head->m_next;
			delete m_head;
			m_head = next;
		}

		m_head = NULL;
		m_tail = NULL;
		m_table.clear();
	}

	//------------------------------------------------------------------------------

	ThreatTable::Entry* 
	ThreatTable::_FindHighestLessThan(
		int32_t			aThreat)
	{
		for(Entry* t = m_head; t != NULL; t = t->m_next)
		{
			if(t->m_threat < aThreat)
				return t;
		}

		return NULL;
	}

	void	
	ThreatTable::_InsertAtEnd(
		Entry*			aEntry)
	{
		aEntry->m_prev = m_tail;

		if(m_tail != NULL)
			m_tail->m_next = aEntry;
		else
			m_head = aEntry;

		m_tail = aEntry;
	}
	
	void	
	ThreatTable::_InsertBefore(
		Entry*			aEntry,
		Entry*			aInsertBefore)
	{
		aEntry->m_next = aInsertBefore;
		aEntry->m_prev = aInsertBefore->m_prev;
		
		if(aInsertBefore->m_prev == NULL)
			m_head = aEntry;
		else
			aInsertBefore->m_prev->m_next = aEntry;

		aInsertBefore->m_prev = aEntry;
	}

	void	
	ThreatTable::_Remove(
		Entry*			aEntry)
	{
		if (aEntry->m_next != NULL)
			aEntry->m_next->m_prev = aEntry->m_prev;
		else
			m_tail = aEntry->m_prev;

		if (aEntry->m_prev != NULL)
			aEntry->m_prev->m_next = aEntry->m_next;
		else
			m_head = aEntry->m_next;

		delete aEntry;
	}

}