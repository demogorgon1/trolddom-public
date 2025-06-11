#include "Pcheader.h"

#include <tpublic/Data/LootCooldown.h>

#include <tpublic/LootCooldowns.h>
#include <tpublic/Manifest.h>
#include <tpublic/TimeSeed.h>

namespace tpublic
{

	void		
	LootCooldowns::ToStream(
		IWriter*		aWriter) const
	{
		aWriter->WriteUIntToUIntTable<uint32_t, uint64_t>(m_table);
	}

	bool		
	LootCooldowns::FromStream(
		IReader*		aReader)
	{
		if(!aReader->ReadUIntToUIntTable<uint32_t, uint64_t>(m_table))
			return false;
		return true;
	}

	void		
	LootCooldowns::Add(
		const Manifest*	aManifest,
		uint32_t		aLootCooldownId)
	{
		uint64_t currentTime = (uint64_t)time(NULL);
		const Data::LootCooldown* lootCooldown = aManifest->GetById<Data::LootCooldown>(aLootCooldownId);

		if(lootCooldown->m_seconds == 0)
		{
			// Fixed daily timer
			uint64_t nextTimeStamp = TimeSeed::GetNextTimeStamp(currentTime, TimeSeed::TYPE_DAILY);

			m_table[aLootCooldownId] = nextTimeStamp;
		}
		else
		{
			m_table[aLootCooldownId] = currentTime + (uint64_t)lootCooldown->m_seconds;
		}
	}
	
	bool		
	LootCooldowns::Check(
		uint32_t		aLootCooldownId,
		uint64_t		aCurrentTime,
		uint64_t*		aOutUntilServerTime) const
	{
		Table::const_iterator i = m_table.find(aLootCooldownId);
		if(i == m_table.cend())
			return false;

		if(aCurrentTime < i->second)
		{
			if(aOutUntilServerTime != NULL)
				*aOutUntilServerTime = i->second;
			return true;
		}

		return false;
	}
	
	bool		
	LootCooldowns::Update()
	{		
		bool dirty = false;

		if(m_table.empty())
		{
			uint64_t currentTime = (uint64_t)time(NULL);

			for (Table::iterator i = m_table.begin(); i != m_table.end();)
			{
				if(currentTime > i->second)
					m_table.erase(i++);
				else
					i++;
			}
		}

		return dirty;
	}
	
	void		
	LootCooldowns::Reset()
	{
		m_table.clear();
	}

}