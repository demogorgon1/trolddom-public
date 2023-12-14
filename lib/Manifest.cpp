#include "Pcheader.h"

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/Achievement.h>
#include <tpublic/Data/AchievementCategory.h>
#include <tpublic/Data/Aura.h>
#include <tpublic/Data/Class.h>
#include <tpublic/Data/CreatureType.h>
#include <tpublic/Data/DialogueRoot.h>
#include <tpublic/Data/DialogueScreen.h>
#include <tpublic/Data/Entity.h>
#include <tpublic/Data/Expression.h>
#include <tpublic/Data/Faction.h>
#include <tpublic/Data/Item.h>
#include <tpublic/Data/LootGroup.h>
#include <tpublic/Data/LootTable.h>
#include <tpublic/Data/Map.h>
#include <tpublic/Data/MapEntitySpawn.h>
#include <tpublic/Data/MapPalette.h>
#include <tpublic/Data/MapPlayerSpawn.h>
#include <tpublic/Data/MapPortal.h>
#include <tpublic/Data/MapSegment.h>
#include <tpublic/Data/MapSegmentConnector.h>
#include <tpublic/Data/MapTrigger.h>
#include <tpublic/Data/NPCBehaviorState.h>
#include <tpublic/Data/Objective.h>
#include <tpublic/Data/ParticleSystem.h>
#include <tpublic/Data/Profession.h>
#include <tpublic/Data/Quest.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/Talent.h>
#include <tpublic/Data/TalentTree.h>
#include <tpublic/Data/Zone.h>

#include <tpublic/Manifest.h>

namespace tpublic
{
	
	Manifest::Manifest()
	{
		RegisterDataContainer<Data::Ability>();
		RegisterDataContainer<Data::Achievement>();
		RegisterDataContainer<Data::AchievementCategory>();
		RegisterDataContainer<Data::Aura>();
		RegisterDataContainer<Data::Class>();
		RegisterDataContainer<Data::CreatureType>();
		RegisterDataContainer<Data::DialogueRoot>();
		RegisterDataContainer<Data::DialogueScreen>();
		RegisterDataContainer<Data::Entity>();
		RegisterDataContainer<Data::Expression>();
		RegisterDataContainer<Data::Faction>();
		RegisterDataContainer<Data::Item>();
		RegisterDataContainer<Data::LootGroup>();
		RegisterDataContainer<Data::LootTable>();
		RegisterDataContainer<Data::Map>();
		RegisterDataContainer<Data::MapEntitySpawn>();
		RegisterDataContainer<Data::MapPalette>();
		RegisterDataContainer<Data::MapPlayerSpawn>();
		RegisterDataContainer<Data::MapPortal>();
		RegisterDataContainer<Data::MapSegment>();
		RegisterDataContainer<Data::MapSegmentConnector>();
		RegisterDataContainer<Data::MapTrigger>();
		RegisterDataContainer<Data::NPCBehaviorState>();
		RegisterDataContainer<Data::Objective>();
		RegisterDataContainer<Data::ParticleSystem>();
		RegisterDataContainer<Data::Profession>();
		RegisterDataContainer<Data::Quest>();
		RegisterDataContainer<Data::Sprite>();
		RegisterDataContainer<Data::Talent>();
		RegisterDataContainer<Data::TalentTree>();
		RegisterDataContainer<Data::Zone>();
	}

	Manifest::~Manifest()
	{

	}

	void
	Manifest::Verify() const
	{
		for (uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
		{
			assert(m_containers[i] != NULL);
			m_containers[i]->Verify();
		}
	}

	void
	Manifest::ToStream(
		IWriter* aStream) const
	{
		for (uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
		{
			assert(m_containers[i] != NULL);
			m_containers[i]->ToStream(aStream);
		}

		m_playerComponents.ToStream(aStream);
		m_xpMetrics.ToStream(aStream);
		m_itemMetrics.ToStream(aStream);
		m_npcMetrics.ToStream(aStream);
		m_questMetrics.ToStream(aStream);
		m_professionMetrics.ToStream(aStream);
		m_abilityMetrics.ToStream(aStream);
	}

	bool
	Manifest::FromStream(
		IReader* aStream)
	{
		for (uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
		{
			assert(m_containers[i] != NULL);
			if (!m_containers[i]->FromStream(aStream))
				return false;
		}

		if (!m_playerComponents.FromStream(aStream))
			return false;
		if (!m_xpMetrics.FromStream(aStream))
			return false;
		if (!m_itemMetrics.FromStream(aStream))
			return false;
		if (!m_npcMetrics.FromStream(aStream))
			return false;
		if (!m_questMetrics.FromStream(aStream))
			return false;
		if (!m_professionMetrics.FromStream(aStream))
			return false;
		if (!m_abilityMetrics.FromStream(aStream))
			return false;

		return true;
	}

	void
	Manifest::PrepareRuntime(
		uint8_t										aRuntime)
	{
		for (uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
		{
			assert(m_containers[i] != NULL);
			m_containers[i]->PrepareRuntime(aRuntime, this);
		}
	}

}