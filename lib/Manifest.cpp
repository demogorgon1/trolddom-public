#include "Pcheader.h"

#include <tpublic/Data/Ability.h>
#include <tpublic/Data/AbilityList.h>
#include <tpublic/Data/AbilityModifier.h>
#include <tpublic/Data/Achievement.h>
#include <tpublic/Data/AchievementCategory.h>
#include <tpublic/Data/Aura.h>
#include <tpublic/Data/AuraGroup.h>
#include <tpublic/Data/Terrain.h>
#include <tpublic/Data/Class.h>
#include <tpublic/Data/CliffStyle.h>
#include <tpublic/Data/ContextHelp.h>
#include <tpublic/Data/ControlPointState.h>
#include <tpublic/Data/Cooldown.h>
#include <tpublic/Data/CreatureType.h>
#include <tpublic/Data/Critter.h>
#include <tpublic/Data/Deity.h>
#include <tpublic/Data/DialogueRoot.h>
#include <tpublic/Data/DialogueScreen.h>
#include <tpublic/Data/Doodad.h>
#include <tpublic/Data/Emote.h>
#include <tpublic/Data/Encounter.h>
#include <tpublic/Data/Entity.h>
#include <tpublic/Data/Expression.h>
#include <tpublic/Data/Faction.h>
#include <tpublic/Data/Item.h>
#include <tpublic/Data/LootCooldown.h>
#include <tpublic/Data/LootGroup.h>
#include <tpublic/Data/LootTable.h>
#include <tpublic/Data/Map.h>
#include <tpublic/Data/MapCliff.h>
#include <tpublic/Data/MapEntitySpawn.h>
#include <tpublic/Data/MapPalette.h>
#include <tpublic/Data/MapPlayerSpawn.h>
#include <tpublic/Data/MapPortal.h>
#include <tpublic/Data/MapSegment.h>
#include <tpublic/Data/MapSegmentConnector.h>
#include <tpublic/Data/MapTrigger.h>
#include <tpublic/Data/MinionMode.h>
#include <tpublic/Data/Mount.h>
#include <tpublic/Data/NameTemplate.h>
#include <tpublic/Data/Noise.h>
#include <tpublic/Data/NPCBehaviorState.h>
#include <tpublic/Data/Objective.h>
#include <tpublic/Data/Pantheon.h>
#include <tpublic/Data/ParticleSystem.h>
#include <tpublic/Data/PlayerWorldType.h>
#include <tpublic/Data/Profession.h>
#include <tpublic/Data/Quest.h>
#include <tpublic/Data/RealmBalance.h>
#include <tpublic/Data/Route.h>
#include <tpublic/Data/Sound.h>
#include <tpublic/Data/Sprite.h>
#include <tpublic/Data/SurvivalScript.h>
#include <tpublic/Data/Tag.h>
#include <tpublic/Data/TagContext.h>
#include <tpublic/Data/Talent.h>
#include <tpublic/Data/TalentTree.h>
#include <tpublic/Data/TileModifier.h>
#include <tpublic/Data/Wall.h>
#include <tpublic/Data/WordGenerator.h>
#include <tpublic/Data/WorldAura.h>
#include <tpublic/Data/Zone.h>

#include <tpublic/Document.h>
#include <tpublic/Manifest.h>

namespace tpublic
{
	
	Manifest::Manifest()
	{
		RegisterDataContainer<Data::Ability>();
		RegisterDataContainer<Data::AbilityList>();
		RegisterDataContainer<Data::AbilityModifier>();
		RegisterDataContainer<Data::Achievement>();
		RegisterDataContainer<Data::AchievementCategory>();
		RegisterDataContainer<Data::Aura>();
		RegisterDataContainer<Data::AuraGroup>();
		RegisterDataContainer<Data::Class>();
		RegisterDataContainer<Data::CliffStyle>();
		RegisterDataContainer<Data::ContextHelp>();
		RegisterDataContainer<Data::ControlPointState>();
		RegisterDataContainer<Data::Cooldown>();
		RegisterDataContainer<Data::CreatureType>();
		RegisterDataContainer<Data::Critter>();
		RegisterDataContainer<Data::Deity>();
		RegisterDataContainer<Data::DialogueRoot>();
		RegisterDataContainer<Data::DialogueScreen>();
		RegisterDataContainer<Data::Doodad>();
		RegisterDataContainer<Data::Emote>();
		RegisterDataContainer<Data::Encounter>();
		RegisterDataContainer<Data::Entity>();
		RegisterDataContainer<Data::Expression>();
		RegisterDataContainer<Data::Faction>();
		RegisterDataContainer<Data::Item>();
		RegisterDataContainer<Data::LootCooldown>();
		RegisterDataContainer<Data::LootGroup>();
		RegisterDataContainer<Data::LootTable>();
		RegisterDataContainer<Data::Map>();
		RegisterDataContainer<Data::MapCliff>();
		RegisterDataContainer<Data::MapEntitySpawn>();
		RegisterDataContainer<Data::MapPalette>();
		RegisterDataContainer<Data::MapPlayerSpawn>();
		RegisterDataContainer<Data::MapPortal>();
		RegisterDataContainer<Data::MapSegment>();
		RegisterDataContainer<Data::MapSegmentConnector>();
		RegisterDataContainer<Data::MapTrigger>();		
		RegisterDataContainer<Data::MinionMode>();
		RegisterDataContainer<Data::Mount>();
		RegisterDataContainer<Data::NameTemplate>();
		RegisterDataContainer<Data::Noise>();
		RegisterDataContainer<Data::NPCBehaviorState>();
		RegisterDataContainer<Data::Objective>();
		RegisterDataContainer<Data::Pantheon>();
		RegisterDataContainer<Data::ParticleSystem>();
		RegisterDataContainer<Data::PlayerWorldType>();
		RegisterDataContainer<Data::Profession>();
		RegisterDataContainer<Data::Quest>();
		RegisterDataContainer<Data::RealmBalance>();
		RegisterDataContainer<Data::Route>();
		RegisterDataContainer<Data::Sound>();
		RegisterDataContainer<Data::Sprite>();
		RegisterDataContainer<Data::SurvivalScript>();
		RegisterDataContainer<Data::Tag>();
		RegisterDataContainer<Data::TagContext>();
		RegisterDataContainer<Data::Talent>();
		RegisterDataContainer<Data::TalentTree>();
		RegisterDataContainer<Data::Terrain>();
		RegisterDataContainer<Data::TileModifier>();
		RegisterDataContainer<Data::Wall>();
		RegisterDataContainer<Data::WordGenerator>();
		RegisterDataContainer<Data::WorldAura>();
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
		m_wordList.ToStream(aStream);
		m_worshipMetrics.ToStream(aStream);
		m_miscMetrics.ToStream(aStream);
		m_reputationMetrics.ToStream(aStream);
		m_defaultSoundEffects.ToStream(aStream);
		m_tileLayering.ToStream(aStream);
		aStream->WriteOptionalObjectPointer(m_changelog);
		aStream->WriteOptionalObjectPointer(m_changelogOld);
		aStream->WriteUInt(m_baseTileBorderPatternSpriteId);
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
		if (!m_wordList.FromStream(aStream))
			return false;
		if (!m_worshipMetrics.FromStream(aStream))
			return false;
		if (!m_miscMetrics.FromStream(aStream))
			return false;
		if (!m_reputationMetrics.FromStream(aStream))
			return false;
		if (!m_defaultSoundEffects.FromStream(aStream))
			return false;
		if (!m_tileLayering.FromStream(aStream))
			return false;
		if(!aStream->ReadOptionalObjectPointer(m_changelog))
			return false;
		if (!aStream->ReadOptionalObjectPointer(m_changelogOld))
			return false;
		if(!aStream->ReadUInt(m_baseTileBorderPatternSpriteId))
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

	const char* 
	Manifest::GetNameByTypeAndId(
		DataType::Id								aDataTypeId,
		uint32_t									aId) const
	{
		if(!DataType::ValidateId(aDataTypeId))
			return "";

		const IDataContainer* container = m_containers[aDataTypeId].get();
		assert(container != NULL);
		const DataBase* base = container->GetExistingBaseById(aId);
		if(base == NULL)
			return "";

		return base->m_name.c_str();
	}

}
