#include "Pcheader.h"

#include <tpublic/ThreatTable.h>

namespace tpublic
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
		uint32_t					aTick,
		std::vector<uint32_t>&		aOutRemoved)
	{
		Entry* t = m_head;
		while(t != NULL)
		{
			Entry* next = t->m_next;

			int32_t ticksSinceLastUpdate = aTick - t->m_tick;

			if(ticksSinceLastUpdate > TIMEOUT_TICKS)
			{
				aOutRemoved.push_back(t->m_entityInstanceId);

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
		Table::iterator i = m_table.find(aEntityInstanceId);
		if(i != m_table.end())
		{
			if(aThreat != 0)
			{
				_Add(i->second, aThreat);

				i->second->m_tick = aTick;
			}
		}
		else
		{
			Entry* entry = new Entry();
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

			entry->m_tick = aTick;
		}
	}

	void			
	ThreatTable::Multiply(
		uint32_t					aTick,
		uint32_t					aEntityInstanceId,
		float						aFactor)
	{
		assert(aFactor > 0.0f);

		Table::iterator i = m_table.find(aEntityInstanceId);
		if (i != m_table.end())
		{
			Entry* entry = i->second;

			int32_t threatChange = 0;

			if(aFactor < 1.0f)
				threatChange = -(int32_t)((float)entry->m_threat * (1.0f - aFactor));
			else if (aFactor > 1.0f)
				threatChange = (int32_t)((float)entry->m_threat * (aFactor - 1.0f));

			_Add(entry, threatChange);

			entry->m_tick = aTick;
		}
	}

	void			
	ThreatTable::MakeTop(
		int32_t						aTick,
		uint32_t					aEntityInstanceId)
	{
		if(m_head != NULL)
		{
			Entry* entry = NULL;
			Table::iterator i = m_table.find(aEntityInstanceId);
			if (i != m_table.end())
				entry = i->second;
			
			if(entry != m_head)
			{
				int32_t topThreat = m_head->m_threat;

				entry->m_threat = topThreat;
				entry->m_tick = aTick;

				_Detach(entry);
				_InsertBefore(entry, m_head);
			}
		}
		else
		{
			Add(aTick, aEntityInstanceId, 1);
		}
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

	void			
	ThreatTable::DebugPrint() const
	{
		for(const Entry* t = GetTop(); t != NULL; t = t->m_next)
			printf("%u: %d threat @ %d tick\n", t->m_entityInstanceId, t->m_threat, t->m_tick);
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
		_Detach(aEntry);

		delete aEntry;
	}

	void	
	ThreatTable::_Detach(
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
	}

	void
	ThreatTable::_Add(
		Entry*			aEntry,
		int32_t			aThreat)
	{
		aEntry->m_threat += aThreat;

		if (aThreat > 0)
		{
			// Move up the list	
			while (aEntry->m_prev != NULL && aEntry->m_prev->m_threat < aEntry->m_threat)
			{
				Entry* prev = aEntry->m_prev;

				if (aEntry->m_next != NULL)
					aEntry->m_next->m_prev = aEntry->m_prev;
				else
					m_tail = aEntry->m_prev;

				if (aEntry->m_prev != NULL)
					aEntry->m_prev->m_next = aEntry->m_next;
				else
					m_head = aEntry->m_next;

				aEntry->m_next = NULL;
				aEntry->m_prev = NULL;

				_InsertBefore(aEntry, prev);
			}
		}
		else
		{
			// Move down the list				
			while (aEntry->m_next != NULL && aEntry->m_next->m_threat > aEntry->m_threat)
			{
				Entry* next = aEntry->m_next;

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

				_InsertBefore(next, aEntry);
			}
		}
	}

}