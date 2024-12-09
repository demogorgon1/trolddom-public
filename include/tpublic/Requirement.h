#pragma once

#include "EntityState.h"
#include "IReader.h"
#include "ItemType.h"
#include "IWriter.h"

namespace tpublic
{

	class Requirement
	{
	public:
		enum Target : uint8_t
		{
			INVALID_TARGET,

			TARGET_TARGET,
			TARGET_SELF
		};

		enum Type : uint8_t
		{
			INVALID_TYPE,

			TYPE_MUST_HAVE_AURA,
			TYPE_MUST_NOT_HAVE_AURA,
			TYPE_MUST_HAVE_COMPLETED_QUEST,
			TYPE_MUST_NOT_HAVE_COMPLETED_QUEST,
			TYPE_MUST_HAVE_ACTIVE_QUEST,
			TYPE_MUST_NOT_HAVE_ACTIVE_QUEST,
			TYPE_MUST_HAVE_TAG,
			TYPE_MUST_BE_TYPE,
			TYPE_MUST_BE_IN_STATE,
			TYPE_MUST_NOT_BE_IN_STATE,
			TYPE_MUST_BE_HOSTILE,
			TYPE_MUST_BE_GROUP_MEMBER,
			TYPE_MUST_HAVE_ITEM_EQUIPPED,
			TYPE_MUST_HAVE_ITEM,
			TYPE_MUST_BE_FACTION,
			TYPE_MUST_HAVE_DISCOVERED_ZONE,
			TYPE_MUST_NOT_HAVE_PROFESSION_ABILITY,
			TYPE_MUST_NOT_HAVE_ITEM_EQUIPPED,
			TYPE_MUST_HAVE_LESS_HEALTH_THAN,
			TYPE_MUST_HAVE_NEGATIVE_REPUTATION,
			TYPE_MUST_HAVE_AURA_GROUP,
			TYPE_MUST_BE_CREATURE_TYPE,
			TYPE_MUST_HAVE_ZERO_HEALTH,
			TYPE_MUST_HAVE_ITEM_TYPE_EQUIPPED,
			TYPE_MUST_HAVE_EQUIPPED_ITEM_TYPE_FLAGS,
			TYPE_MUST_BE_AT_LEAST_LEVEL,
			TYPE_MUST_BE_DISCIPLE,
			TYPE_MUST_HAVE_MORE_RAGE_THAN,
			TYPE_MUST_NOT_BE_READY_TO_TURN_IN_QUEST
		};

		static DataType::Id
		GetDataType(
			Type					aType)
		{
			switch (aType)
			{
			case TYPE_MUST_HAVE_AURA:
			case TYPE_MUST_NOT_HAVE_AURA:
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
				return DataType::ID_ENTITY;

			case TYPE_MUST_NOT_HAVE_ITEM_EQUIPPED:
			case TYPE_MUST_HAVE_ITEM_EQUIPPED:
			case TYPE_MUST_HAVE_ITEM:
				return DataType::ID_ITEM;

			case TYPE_MUST_BE_FACTION:
			case TYPE_MUST_HAVE_NEGATIVE_REPUTATION:
			case TYPE_MUST_BE_DISCIPLE:
				return DataType::ID_FACTION;

			case TYPE_MUST_HAVE_DISCOVERED_ZONE:
				return DataType::ID_ZONE;

			case TYPE_MUST_NOT_HAVE_PROFESSION_ABILITY:
				return DataType::ID_ABILITY;

			case TYPE_MUST_HAVE_AURA_GROUP:
				return DataType::ID_AURA_GROUP;

			case TYPE_MUST_BE_CREATURE_TYPE:
				return DataType::ID_CREATURE_TYPE;

			case TYPE_MUST_HAVE_LESS_HEALTH_THAN:
			case TYPE_MUST_HAVE_MORE_RAGE_THAN:
				return DataType::INVALID_ID;

			default:
				break;
			}
			assert(false);
			return DataType::INVALID_ID;
		}

		static uint32_t 
		TypeAndSourceToId(
			Type				aType,
			const SourceNode*	aSource)
		{
			if (aType == TYPE_MUST_BE_IN_STATE || aType == TYPE_MUST_NOT_BE_IN_STATE)
			{
				EntityState::Id entityState = EntityState::StringToId(aSource->GetIdentifier());
				TP_VERIFY(entityState != EntityState::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid entity state.", aSource->GetIdentifier());
				return (uint32_t)entityState;
			}
			else if(aType == TYPE_MUST_HAVE_ITEM_TYPE_EQUIPPED)
			{
				ItemType::Id itemType = ItemType::StringToId(aSource->GetIdentifier());
				TP_VERIFY(itemType != ItemType::INVALID_ID, aSource->m_debugInfo, "'%s' is not a valid item type.", aSource->GetIdentifier());
				return (uint32_t)itemType;
			}
			else if(aType == TYPE_MUST_HAVE_EQUIPPED_ITEM_TYPE_FLAGS)
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

			return aSource->m_sourceContext->m_persistentIdTable->GetId(GetDataType(aType), aSource->GetIdentifier());
		}

		Requirement()
		{

		}

		Requirement(
			const SourceNode*	aSource)
		{
			FromSource(aSource);
		}

		void
		FromSource(
			const SourceNode*	aSource)
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
			else if(typeString == "must_have_zero_health")
				m_type = TYPE_MUST_HAVE_ZERO_HEALTH;
			else if (typeString == "must_have_item_type_equipped")
				m_type = TYPE_MUST_HAVE_ITEM_TYPE_EQUIPPED;
			else if (typeString == "must_have_equipped_item_type_flags")
				m_type = TYPE_MUST_HAVE_EQUIPPED_ITEM_TYPE_FLAGS;
			else if (typeString == "must_be_at_least_level")
				m_type = TYPE_MUST_BE_AT_LEAST_LEVEL;
			else if(typeString == "must_be_disciple")
				m_type = TYPE_MUST_BE_DISCIPLE;
			else if(typeString == "must_not_be_ready_to_turn_in_quest")
				m_type = TYPE_MUST_NOT_BE_READY_TO_TURN_IN_QUEST;
			else
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid type.", aSource->m_annotation->GetIdentifier());

			if(aSource->m_type == SourceNode::TYPE_NUMBER)
			{
				m_id = aSource->GetUInt32();
			}
			else if(aSource->m_type == SourceNode::TYPE_IDENTIFIER)
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
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}
		}

		void
		ToStream(
			IWriter*			aWriter) const
		{
			aWriter->WritePOD(m_target);
			aWriter->WritePOD(m_type);
			aWriter->WriteUInt(m_id);
		}

		bool
		FromStream(
			IReader*			aReader)
		{
			if (!aReader->ReadPOD(m_target))
				return false;
			if (!aReader->ReadPOD(m_type))
				return false;
			if (!aReader->ReadUInt(m_id))
				return false;
			return true;
		}

		// Public data
		Target								m_target = INVALID_TARGET;
		Type								m_type = INVALID_TYPE;
		uint32_t							m_id = 0;
	};

}