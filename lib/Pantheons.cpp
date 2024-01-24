#include "Pcheader.h"

#include <tpublic/Data/Deity.h>
#include <tpublic/Data/Pantheon.h>

#include <tpublic/Manifest.h>
#include <tpublic/Pantheons.h>

namespace tpublic
{

	Pantheons::Pantheons(
		const Manifest*			aManifest)
	{
		aManifest->GetContainer<Data::Pantheon>()->ForEach([&](
			const Data::Pantheon* aPantheon)
		{
			assert(!m_table.contains(aPantheon->m_id));
			std::unique_ptr<Entry> entry = std::make_unique<Entry>();
			entry->m_pantheon = aPantheon;
			m_table[aPantheon->m_id] = std::move(entry);
			return true;
		});

		aManifest->GetContainer<Data::Deity>()->ForEach([&](
			const Data::Deity* aDeity)
		{
			Table::iterator i = m_table.find(aDeity->m_pantheonId);
			TP_CHECK(i != m_table.end(), "Invalid pantheon id: %u", aDeity->m_pantheonId);
			Entry* entry = i->second.get();
			entry->m_deities.push_back(aDeity);
			return true;
		});
	}
	
	Pantheons::~Pantheons()
	{

	}

	const Pantheons::Entry* 
	Pantheons::GetEntry(
		uint32_t				aPantheonId) const
	{
		Table::const_iterator i = m_table.find(aPantheonId);
		if(i == m_table.cend())
			return NULL;
		return i->second.get();
	}

}