#include "Pcheader.h"

#include <tpublic/Data/Aura.h>

#include <tpublic/Requirement.h>

namespace tpublic
{

	DataType::Id
	Requirement::GetDataType(
		Type							aType)
	{
		switch (aType)
		{
		case TYPE_MUST_HAVE_AURA:
		case TYPE_MUST_NOT_HAVE_AURA:
		case TYPE_MUST_NOT_HAVE_SOURCE_AURA:
			return DataType::ID_AURA;

		case TYPE_MUST_HAVE_COMPLETED_QUEST:
		case TYPE_MUST_NOT_HAVE_COMPLETED_QUEST:
		case TYPE_MUST_HAVE_ACTIVE_QUEST:
		case TYPE_MUST_NOT_HAVE_ACTIVE_QUEST:
		case TYPE_MUST_NOT_BE_READY_TO_TURN_IN_QUEST:
			return DataType::ID_QUEST;

		case TYPE_MUST_HAVE_TAG:
			return DataType::ID_TAG;

		case TYPE_MUST_BE_TYPE:
		case TYPE_MUST_NOT_BE_TYPE:
			return DataType::ID_ENTITY;

		case TYPE_MUST_NOT_HAVE_ITEM_EQUIPPED:
		case TYPE_MUST_HAVE_ITEM_EQUIPPED:
		case TYPE_MUST_NOT_HAVE_ITEM:
		case TYPE_MUST_HAVE_ITEM:
			return DataType::ID_ITEM;

		case TYPE_MUST_BE_FACTION:
		case TYPE_MUST_HAVE_NEGATIVE_REPUTATION:
		case TYPE_MUST_BE_DISCIPLE:
		case TYPE_MUST_NOT_BE_DISCIPLE:
		case TYPE_MUST_HAVE_REPUTATION_LEVEL:
			return DataType::ID_FACTION;

		case TYPE_MUST_HAVE_DISCOVERED_ZONE:
		case TYPE_MUST_NOT_HAVE_DISCOVERED_ZONE:
			return DataType::ID_ZONE;

		case TYPE_MUST_NOT_HAVE_PROFESSION_ABILITY:
			return DataType::ID_ABILITY;

		case TYPE_MUST_HAVE_AURA_GROUP:
			return DataType::ID_AURA_GROUP;

		case TYPE_MUST_BE_CREATURE_TYPE:
		case TYPE_MUST_NOT_BE_CREATURE_TYPE:
			return DataType::ID_CREATURE_TYPE;

		case TYPE_MUST_HAVE_LESS_HEALTH_THAN:
		case TYPE_MUST_HAVE_MORE_RAGE_THAN:
			return DataType::INVALID_ID;

		case TYPE_MUST_BE_ON_MAP:
		case TYPE_MUST_NOT_BE_ON_MAP:
			return DataType::ID_MAP;

		default:
			break;
		}
		assert(false);
		return DataType::INVALID_ID;
	}

	uint32_t
	Requirement::TypeAndSourceToId(
		Type							aType,
		const SourceNode*				aSource)
	{
		if (aType == TYPE_MUST_BE_IN_STATE || aType == TYPE_MUST_NOT_BE_IN_STATE)
		{
			EntityState::Id entityState = EntityState::StringToId(aSource->GetIdentifier());
			TP_VERIFY(entityState != EntityState::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid entity state.", aSource->GetIdentifier());
			return (uint32_t)entityState;
		}
		else if (aType == TYPE_MUST_HAVE_ITEM_TYPE_EQUIPPED)
		{
			ItemType::Id itemType = ItemType::StringToId(aSource->GetIdentifier());
			TP_VERIFY(itemType != ItemType::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid item type.", aSource->GetIdentifier());
			return (uint32_t)itemType;
		}
		else if (aType == TYPE_MUST_HAVE_EQUIPPED_ITEM_TYPE_FLAGS || aType == TYPE_MUST_NOT_HAVE_EQUIPPED_ITEM_TYPE_FLAGS)
		{
			uint16_t flags = 0;
			aSource->GetArray()->ForEachChild([&](
				const SourceNode* aChild)
			{
				uint16_t flag = ItemType::StringToFlag(aChild->GetIdentifier());
				TP_VERIFY(flag != 0, aChild->m_debugInfo, "'%s' is not a valid item type flag.", aChild->GetIdentifier());
				flags |= flag;
			});
			return (uint32_t)flags;
		}
		else if (aType == TYPE_MUST_HAVE_AURA_FLAGS)
		{
			return Data::Aura::SourceToFlags(aSource);
		}

		return aSource->GetId(GetDataType(aType));
	}

	//--------------------------------------------------------------------------------

	Requirement::Requirement()
	{

	}

	Requirement::Requirement(
		const SourceNode*				aSource)
	{
		FromSource(aSource);
	}

	void
	Requirement::FromSource(
		const SourceNode*				aSource)
	{
		if (aSource->m_name == "target")
			m_target = TARGET_TARGET;
		else if (aSource->m_name == "self")
			m_target = TARGET_SELF;
		else
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid target.", aSource->m_name.c_str());

		TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing type annotation.");
		std::string_view typeString(aSource->m_annotation->GetIdentifier());
		if (typeString == "must_have_aura")
			m_type = TYPE_MUST_HAVE_AURA;
		else if (typeString == "must_not_have_aura")
			m_type = TYPE_MUST_NOT_HAVE_AURA;
		else if (typeString == "must_have_completed_quest")
			m_type = TYPE_MUST_HAVE_COMPLETED_QUEST;
		else if (typeString == "must_not_have_completed_quest")
			m_type = TYPE_MUST_NOT_HAVE_COMPLETED_QUEST;
		else if (typeString == "must_have_active_quest")
			m_type = TYPE_MUST_HAVE_ACTIVE_QUEST;
		else if (typeString == "must_not_have_active_quest")
			m_type = TYPE_MUST_NOT_HAVE_ACTIVE_QUEST;
		else if (typeString == "must_have_tag")
			m_type = TYPE_MUST_HAVE_TAG;
		else if (typeString == "must_be_type")
			m_type = TYPE_MUST_BE_TYPE;
		else if (typeString == "must_not_be_type")
			m_type = TYPE_MUST_NOT_BE_TYPE;
		else if (typeString == "must_be_in_state")
			m_type = TYPE_MUST_BE_IN_STATE;
		else if (typeString == "must_not_be_in_state")
			m_type = TYPE_MUST_NOT_BE_IN_STATE;
		else if (typeString == "must_be_hostile")
			m_type = TYPE_MUST_BE_HOSTILE;
		else if (typeString == "must_be_group_member")
			m_type = TYPE_MUST_BE_GROUP_MEMBER;
		else if (typeString == "must_have_item_equipped")
			m_type = TYPE_MUST_HAVE_ITEM_EQUIPPED;
		else if (typeString == "must_not_have_item_equipped")
			m_type = TYPE_MUST_NOT_HAVE_ITEM_EQUIPPED;
		else if (typeString == "must_have_item")
			m_type = TYPE_MUST_HAVE_ITEM;
		else if (typeString == "must_be_faction")
			m_type = TYPE_MUST_BE_FACTION;
		else if (typeString == "must_have_discovered_zone")
			m_type = TYPE_MUST_HAVE_DISCOVERED_ZONE;
		else if (typeString == "must_not_have_discovered_zone")
			m_type = TYPE_MUST_NOT_HAVE_DISCOVERED_ZONE;
		else if (typeString == "must_not_have_profession_ability")
			m_type = TYPE_MUST_NOT_HAVE_PROFESSION_ABILITY;
		else if (typeString == "must_have_less_health_than")
			m_type = TYPE_MUST_HAVE_LESS_HEALTH_THAN;
		else if (typeString == "must_have_more_rage_than")
			m_type = TYPE_MUST_HAVE_MORE_RAGE_THAN;
		else if (typeString == "must_have_negative_reputation")
			m_type = TYPE_MUST_HAVE_NEGATIVE_REPUTATION;
		else if (typeString == "must_have_aura_group")
			m_type = TYPE_MUST_HAVE_AURA_GROUP;
		else if (typeString == "must_be_creature_type")
			m_type = TYPE_MUST_BE_CREATURE_TYPE;
		else if (typeString == "must_not_be_creature_type")
			m_type = TYPE_MUST_NOT_BE_CREATURE_TYPE;
		else if (typeString == "must_have_zero_health")
			m_type = TYPE_MUST_HAVE_ZERO_HEALTH;
		else if (typeString == "must_have_item_type_equipped")
			m_type = TYPE_MUST_HAVE_ITEM_TYPE_EQUIPPED;
		else if (typeString == "must_have_equipped_item_type_flags")
			m_type = TYPE_MUST_HAVE_EQUIPPED_ITEM_TYPE_FLAGS;
		else if (typeString == "must_not_have_equipped_item_type_flags")
			m_type = TYPE_MUST_NOT_HAVE_EQUIPPED_ITEM_TYPE_FLAGS;
		else if (typeString == "must_be_at_least_level")
			m_type = TYPE_MUST_BE_AT_LEAST_LEVEL;
		else if (typeString == "must_be_disciple")
			m_type = TYPE_MUST_BE_DISCIPLE;
		else if (typeString == "must_not_be_disciple")
			m_type = TYPE_MUST_NOT_BE_DISCIPLE;
		else if (typeString == "must_not_be_ready_to_turn_in_quest")
			m_type = TYPE_MUST_NOT_BE_READY_TO_TURN_IN_QUEST;
		else if (typeString == "must_not_have_item")
			m_type = TYPE_MUST_NOT_HAVE_ITEM;
		else if (typeString == "must_have_reputation_level")
			m_type = TYPE_MUST_HAVE_REPUTATION_LEVEL;
		else if (typeString == "must_know_riding")
			m_type = TYPE_MUST_KNOW_RIDING;
		else if (typeString == "must_not_know_riding")
			m_type = TYPE_MUST_NOT_KNOW_RIDING;
		else if (typeString == "must_not_have_started_survival")
			m_type = TYPE_MUST_NOT_HAVE_STARTED_SURVIVAL;
		else if (typeString == "must_have_started_survival")
			m_type = TYPE_MUST_HAVE_STARTED_SURVIVAL;
		else if (typeString == "must_not_be_on_map")
			m_type = TYPE_MUST_NOT_BE_ON_MAP;
		else if (typeString == "must_be_on_map")
			m_type = TYPE_MUST_BE_ON_MAP;
		else if (typeString == "must_have_fishing_rod_equipped")
			m_type = TYPE_MUST_HAVE_FISHING_ROD_EQUIPPED;
		else if (typeString == "must_not_have_source_aura")
			m_type = TYPE_MUST_NOT_HAVE_SOURCE_AURA;
		else if (typeString == "must_have_aura_flags")
			m_type = TYPE_MUST_HAVE_AURA_FLAGS;
		else if (typeString == "must_be_elite")
			m_type = TYPE_MUST_BE_ELITE;
		else if (typeString == "must_not_be_elite")
			m_type = TYPE_MUST_NOT_BE_ELITE;
		else if(typeString == "must_be_lower_than_level")
			m_type = TYPE_MUST_BE_LOWER_THAN_LEVEL;
		else if(typeString == "must_be_npc_with_head_anchor")
			m_type = TYPE_MUST_BE_NPC_WITH_HEAD_ANCHOR;
		else if (typeString == "must_be_heroic")
			m_type = TYPE_MUST_BE_HEROIC;
		else if (typeString == "must_not_be_heroic")
			m_type = TYPE_MUST_NOT_BE_HEROIC;
		else if(typeString == "must_not_be_friendly_faction")
			m_type = TYPE_MUST_NOT_BE_FRIENDLY_FACTION;
		else
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid type.", aSource->m_annotation->GetIdentifier());

		if (aSource->m_type == SourceNode::TYPE_NUMBER)
		{
			m_id = aSource->GetUInt32();
		}
		else if (aSource->m_type == SourceNode::TYPE_IDENTIFIER || aSource->m_type == SourceNode::TYPE_ARRAY)
		{
			m_id = TypeAndSourceToId(m_type, aSource);
		}
		else
		{
			aSource->GetObject()->ForEachChild([&](
				const SourceNode* aChild)
			{
				if (aChild->m_name == "id")
					m_id = TypeAndSourceToId(m_type, aChild);
				else if (aChild->m_name == "value")
					m_value = aChild->GetUInt32();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}
	}

	void
	Requirement::ToStream(
		IWriter*			aWriter) const
	{
		aWriter->WritePOD(m_target);
		aWriter->WritePOD(m_type);
		aWriter->WriteUInt(m_id);
		aWriter->WriteUInt(m_value);
	}

	bool
	Requirement::FromStream(
		IReader*			aReader)
	{
		if (!aReader->ReadPOD(m_target))
			return false;
		if (!aReader->ReadPOD(m_type))
			return false;
		if (!aReader->ReadUInt(m_id))
			return false;
		if (!aReader->ReadUInt(m_value))
			return false;
		return true;
	}

}