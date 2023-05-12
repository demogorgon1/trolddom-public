#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class ActionBar
	{
	public:
		static const size_t MAX_SIZE = 10;

		void
		SetSlot(
			size_t				aIndex,
			uint32_t			aAbilityId)
		{
			assert(aIndex < MAX_SIZE);
			if(aIndex >= m_slots.size())
				m_slots.resize(aIndex + 1);
			m_slots[aIndex] = aAbilityId;

			size_t trailingEmptySlots = 0;
			for (std::vector<uint32_t>::reverse_iterator i = m_slots.rbegin(); i != m_slots.rend() && *i == 0; i++)
				trailingEmptySlots++;
			assert(trailingEmptySlots <= m_slots.size());
			m_slots.resize(m_slots.size() - trailingEmptySlots);
		}

		ActionBar*
		Copy() const
		{
			ActionBar* t = new ActionBar();
			t->m_slots = m_slots;
			return t;
		}

		void
		ToStream(
			IWriter*			aStream) const
		{
			aStream->WriteUInts(m_slots);
		}

		bool
		FromStream(
			IReader*			aStream) 
		{
			if(!aStream->ReadUInts(m_slots))
				return false;
			return true;
		}

		// Public data
		std::vector<uint32_t>		m_slots;
	};

}