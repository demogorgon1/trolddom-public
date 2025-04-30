#pragma once

namespace tpublic
{

	namespace Components
	{
		struct CombatPublic;
		struct PlayerPublic;
		struct Talents;
		struct EquippedItems;
	}

	class IReader;
	class IWriter;

	class CharacterDigest
	{
	public:
		std::string	AsBase64() const;
		void		FromBase64(
						const char*							aBase64);
		void		ToStream(
						IWriter*							aWriter) const;
		bool		FromStream(
						IReader*							aReader);
		void		Construct(
						const Components::CombatPublic*		aCombatPublic,
						const Components::PlayerPublic*		aPlayerPublic,
						const Components::Talents*			aTalents,
						const Components::EquippedItems*	aEquippedItems,
						uint32_t							aClassId);
	 
		// Public data
		Stat::Collection						m_stats;
		std::vector<uint32_t>					m_equippedItemIds;
		uint32_t								m_level = 0;
		uint32_t								m_classId = 0;
		uint32_t								m_maxHealth = 0;
		std::unordered_map<uint32_t, uint32_t>	m_talentPoints;
	};

}