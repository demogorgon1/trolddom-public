#include "Pcheader.h"

#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/EquippedItems.h>
#include <tpublic/Components/PlayerPublic.h>
#include <tpublic/Components/Talents.h>

#include <tpublic/CharacterDigest.h>
#include <tpublic/IReader.h>
#include <tpublic/IWriter.h>

namespace tpublic
{

	std::string	
	CharacterDigest::AsBase64() const
	{
		std::vector<uint8_t> buffer;
		VectorIO::Writer writer(buffer);
		ToStream(&writer);
		std::string base64;
		Base64::Encode(&buffer[0], buffer.size(), base64);
		return base64;
	}
	
	void
	CharacterDigest::FromBase64(
		const char*							aBase64)
	{
		std::vector<uint8_t> buffer;
		Base64::Decode(aBase64, buffer);
		VectorIO::Reader reader(buffer);
		bool ok = FromStream(&reader);
		TP_CHECK(ok, "Failed to read character digest.");		
	}
	
	void		
	CharacterDigest::ToStream(
		IWriter*							aWriter) const
	{
		m_stats.ToStream(aWriter);
		aWriter->WriteUInts(m_equippedItemIds);
		aWriter->WriteUInt(m_level);
		aWriter->WriteUInt(m_classId);
		aWriter->WriteUInt(m_maxHealth);
		aWriter->WriteUIntToUIntTable<uint32_t, uint32_t>(m_talentPoints);
	}
	
	bool		
	CharacterDigest::FromStream(
		IReader*							aReader)
	{
		if(!m_stats.FromStream(aReader))
			return false;
		if(!aReader->ReadUInts(m_equippedItemIds))
			return false;
		if (!aReader->ReadUInt(m_level))
			return false;
		if (!aReader->ReadUInt(m_classId))
			return false;
		if (!aReader->ReadUInt(m_maxHealth))
			return false;
		if(!aReader->ReadUIntToUIntTable(m_talentPoints))
			return false;
		return true;
	}
	
	void		
	CharacterDigest::Construct(
		const Components::CombatPublic*		aCombatPublic,
		const Components::PlayerPublic*		aPlayerPublic,
		const Components::Talents*			aTalents,
		const Components::EquippedItems*	aEquippedItems,
		uint32_t							aClassId)
	{
		m_stats = aPlayerPublic->m_stats;
		m_level = aCombatPublic->m_level;
		m_classId = aClassId;
		m_maxHealth = aCombatPublic->GetResourceMax(Resource::ID_HEALTH);
		
		m_equippedItemIds.clear();
		
		aEquippedItems->ForEach([&](
			const ItemInstance& aItemInstance)
		{
			if(aItemInstance.IsSet())
				m_equippedItemIds.push_back(aItemInstance.m_itemId);
		});

		m_talentPoints.clear();
		for(const Components::Talents::Entry& t : aTalents->m_entries)
			m_talentPoints[t.m_talentId] = t.m_points;
	}

}