#include "Pcheader.h"

#include <tpublic/Helpers.h>
#include <tpublic/ThreatTable.h>

namespace tpublic
{

	ThreatTable::ThreatTable()
		: m_head(NULL)
		, m_tail(NULL)
		, m_tick(0)
	{

	}
	
	ThreatTable::~ThreatTable()
	{
		Clear();
	}

	void	
	ThreatTable::Update(
		int32_t								aTick,
		std::vector<SourceEntityInstance>&	aOutRemoved)
	{
		if(m_head == NULL)
			return;

		int32_t ticksSinceLastEvent = aTick - m_tick;
		if (ticksSinceLastEvent > TIMEOUT_TICKS)
		{
			while (m_head != NULL)
			{
				aOutRemoved.push_back(m_head->m_key);

				Entry* next = m_head->m_next;
				delete m_head;
				m_head = next;
			}

			m_head = NULL;
			m_tail = NULL;
			m_table.clear();
			m_tick = 0;
		}
	}
	
	void	
	ThreatTable::Add(
		int32_t								aTick,
		const SourceEntityInstance&			aSourceEntityInstance,
		int32_t								aThreat)
	{
		Table::iterator i = m_table.find(aSourceEntityInstance);
		if(i != m_table.end())
		{
			if(aThreat != 0)
				_Add(i->second, aThreat);
		}
		else
		{
			Entry* entry = new Entry();
			m_table[aSourceEntityInstance] = entry;

			entry->m_key = aSourceEntityInstance;
			entry->m_threat = aThreat;
			if(entry->m_threat < 0)
				entry->m_threat = 0;

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

		if(aTick > m_tick)
			m_tick = aTick;

		FastValidate();
	}

	int32_t
	ThreatTable::Multiply(
		int32_t						aTick,
		const SourceEntityInstance& aSourceEntityInstance,
		float						aFactor)
	{
		assert(aFactor > 0.0f);

		int32_t returnValue = INT32_MAX;

		Table::iterator i = m_table.find(aSourceEntityInstance);
		if (i != m_table.end())
		{
			Entry* entry = i->second;

			int32_t threatChange = 0;

			if(aFactor < 1.0f)
				threatChange = -(int32_t)((float)entry->m_threat * (1.0f - aFactor));
			else if (aFactor > 1.0f)
				threatChange = (int32_t)((float)entry->m_threat * (aFactor - 1.0f));

			_Add(entry, threatChange);

			returnValue = threatChange;
		}

		if (aTick > m_tick)
			m_tick = aTick;

		FastValidate();

		return returnValue;
	}

	void			
	ThreatTable::MakeTop(
		int32_t						aTick,
		const SourceEntityInstance& aSourceEntityInstanceq)
	{
		if(m_head != NULL)
		{
			Entry* entry = NULL;
			Table::iterator i = m_table.find(aSourceEntityInstanceq);
			if (i != m_table.end())
				entry = i->second;

			assert(m_table.size() > 0);

			if (entry != m_head)
			{
				int32_t topThreat = m_head->m_threat + 1;

				if(entry == NULL)
				{
					Add(aTick, aSourceEntityInstanceq, topThreat);
				}
				else if(entry->m_threat < m_head->m_threat)
				{
					entry->m_threat = topThreat;

					_Detach(entry);
					_InsertBefore(entry, m_head);
				}
			}
		}
		else
		{
			Add(aTick, aSourceEntityInstanceq, 1);
		}

		if (aTick > m_tick)
			m_tick = aTick;

		FastValidate();
	}

	void	
	ThreatTable::Remove(
		const SourceEntityInstance& aSourceEntityInstance)
	{
		Table::iterator i = m_table.find(aSourceEntityInstance);
		if (i != m_table.end())
		{
			_Remove(i->second);

			m_table.erase(i);
		}

		FastValidate();
	}

	void			
	ThreatTable::Touch(
		int32_t									aTick)
	{
		if (aTick > m_tick)
		{
			// Only update tick if we have any threat sources with more than 0 threat
			bool hasNonZeroSources = false;
			for(const Entry* t = m_head; t != NULL && !hasNonZeroSources; t = t->m_next)
				hasNonZeroSources = t->m_threat > 0;

			if(hasNonZeroSources)
				m_tick = aTick;
		}
	}

	int32_t			
	ThreatTable::GetThreat(
		const SourceEntityInstance&				aSourceEntityInstance) const
	{
		Table::const_iterator i = m_table.find(aSourceEntityInstance);
		if(i == m_table.cend())
			return 0;
		return i->second->m_threat;
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
		m_tick = 0;
	}

	void			
	ThreatTable::Equalize()
	{
		for (Entry* t = m_head; t != NULL; t = t->m_next)
			t->m_threat = 0;
	}
	
	void			
	ThreatTable::Shuffle(
		uint32_t								aSeed)
	{
		std::vector<Entry*> tmp;
		for (Entry* t = m_head; t != NULL; t = t->m_next)
		{
			t->m_next = NULL;
			t->m_prev = NULL;
			tmp.push_back(t);
		}
		m_head = NULL;
		m_tail = NULL;
		uint32_t r = aSeed;

		while(tmp.size() > 0)
		{
			UniformDistribution<uint32_t> distribution(0, (uint32_t)tmp.size() - 1);
			uint32_t i = distribution.Generate(r);
			_InsertAtEnd(tmp[i]);
			Helpers::RemoveCyclicFromVector(tmp, i);

			// This is a bit hackish, but we don't have (easy) access to a std::mt19937 here
			r = (uint32_t)Hash::Splitmix_2_32(r, r);
		}
	}

	void			
	ThreatTable::DebugPrint() const
	{
		printf("(tick %d)\n", m_tick);
		for(const Entry* t = GetTop(); t != NULL; t = t->m_next)
			printf("%u[%u]: %d threat\n", t->m_key.m_entityInstanceId, t->m_key.m_entityInstanceSeq, t->m_threat);
	}

	void			
	ThreatTable::GetEntityInstanceIds(
		std::vector<uint32_t>& aOutEntityInstanceIds) const
	{
		for (const Entry* t = GetTop(); t != NULL; t = t->m_next)
			aOutEntityInstanceIds.push_back(t->m_key.m_entityInstanceId);
	}

	void			
	ThreatTable::Validate() const
	{
		FastValidate();

		int32_t prevThreat = INT32_MAX;
		std::unordered_set<SourceEntityInstance, SourceEntityInstance::Hasher> entitySet;

		for (const Entry* t = GetTop(); t != NULL; t = t->m_next)
		{
			assert(!entitySet.contains(t->m_key));
			assert(t->m_threat <= prevThreat);
			TP_UNUSED(prevThreat);

			entitySet.insert(t->m_key);
			prevThreat = t->m_threat;
		}

		assert(entitySet.size() == m_table.size());
	}

	void			
	ThreatTable::FastValidate() const
	{
		if(m_head == NULL)
		{
			assert(m_tail == NULL);
			assert(m_table.size() == 0);
		}
		else
		{
			assert(m_head->m_prev == NULL);
		}
		
		if(m_tail == NULL)
		{
			assert(m_head == NULL);
			assert(m_table.size() == 0);
		}
		else
		{
			assert(m_tail->m_next == NULL);
		}

		if(m_table.size() == 0)
		{
			assert(m_head == NULL);
			assert(m_tail == NULL);
		}
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
		
		if(aEntry->m_threat < 0)
			aEntry->m_threat = 0;

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