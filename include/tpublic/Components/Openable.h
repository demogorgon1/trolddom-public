#pragma once

#include "../Chat.h"
#include "../Component.h"
#include "../ComponentBase.h"
#include "../Requirement.h"

namespace tpublic
{

	namespace Components
	{

		struct Openable
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_OPENABLE;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			struct Verb
			{
				enum Type : uint8_t
				{
					TYPE_OPEN,
					TYPE_PICK,
					TYPE_MINE,
					TYPE_CATCH,
					TYPE_PICK_UP,
					TYPE_INSPECT,
					TYPE_TOUCH,
					TYPE_DESTROY,
					TYPE_BURN,
					TYPE_WATER,
					TYPE_USE,
					TYPE_RING,
					TYPE_CAPTURE
				};

				void
				FromSource(
					const SourceNode*	aSource)
				{
					std::string_view t(aSource->GetIdentifier());
					if (t == "open")
						m_type = TYPE_OPEN;
					else if (t == "pick")
						m_type = TYPE_PICK;
					else if (t == "mine")
						m_type = TYPE_MINE;
					else if (t == "catch")
						m_type = TYPE_CATCH;
					else if (t == "pick_up")
						m_type = TYPE_PICK_UP;
					else if (t == "inspect")
						m_type = TYPE_INSPECT;
					else if (t == "touch")
						m_type = TYPE_TOUCH;
					else if (t == "destroy")
						m_type = TYPE_DESTROY;
					else if (t == "burn")
						m_type = TYPE_BURN;
					else if (t == "water")
						m_type = TYPE_WATER;
					else if (t == "use")
						m_type = TYPE_USE;
					else if (t == "ring")
						m_type = TYPE_RING;
					else if (t == "capture")
						m_type = TYPE_CAPTURE;
					else
						TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid verb.", aSource->GetIdentifier());
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WritePOD(m_type);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					return true;
				}

				const char*
				GetDisplayString() const
				{
					switch(m_type)
					{
					case TYPE_OPEN:		return "Open";
					case TYPE_PICK:		return "Pick";
					case TYPE_MINE:		return "Mine";
					case TYPE_CATCH:	return "Catch";
					case TYPE_PICK_UP:	return "Pick Up";
					case TYPE_INSPECT:	return "Inspect";
					case TYPE_TOUCH:	return "Touch";
					case TYPE_DESTROY:	return "Destroy";
					case TYPE_BURN:		return "Burn";
					case TYPE_WATER:	return "Water";
					case TYPE_USE:		return "Use";
					case TYPE_RING:		return "Ring";
					case TYPE_CAPTURE:	return "Capture";
					default:			break;
					}
					assert(false);
					return "";
				}

				// Public data
				Type				m_type = TYPE_OPEN;
			};

			enum Field
			{
				FIELD_LOOT_TABLE_ID,
				FIELD_REQUIRED_ITEM_ID,
				FIELD_VERB,
				FIELD_REQUIRED_PROFESSION_ID,
				FIELD_REQUIRED_PROFESSION_SKILL,
				FIELD_RANGE,
				FIELD_DESPAWN,
				FIELD_OPENED,
				FIELD_INSTANT,
				FIELD_DURATION,
				FIELD_LEVEL,
				FIELD_REQUIREMENTS,
				FIELD_SOUND,
				FIELD_REQUIRED_QUEST_ID,
				FIELD_COMPLETE_MANUAL_OBJECTIVE_ID,
				FIELD_TRIGGER_ABILITY_ID,
				FIELD_REQUIRED_INCOMPLETE_QUEST_OBJECTIVE_ID,
				FIELD_UNLOCK_MAP_TRIGGER_ID,
				FIELD_KILL,
				FIELD_DEAD_DESPAWN_TICKS,
				FIELD_CONTEXT_HELP,
				FIELD_SPRITE_INDEX,
				FIELD_CHATS
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LOOT_TABLE_ID, "loot_table", offsetof(Openable, m_lootTableId))->SetDataType(DataType::ID_LOOT_TABLE);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_REQUIRED_ITEM_ID, "required_item", offsetof(Openable, m_requiredItemId))->SetDataType(DataType::ID_ITEM);
				aSchema->DefineCustomObject<Verb>(FIELD_VERB, "verb", offsetof(Openable, m_verb));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_REQUIRED_PROFESSION_ID, "required_profession", offsetof(Openable, m_requiredProfessionId))->SetDataType(DataType::ID_PROFESSION);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_REQUIRED_PROFESSION_SKILL, "required_profession_skill", offsetof(Openable, m_requiredProfessionSkill));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_REQUIRED_QUEST_ID, "required_quest", offsetof(Openable, m_requiredQuestId))->SetDataType(DataType::ID_QUEST);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_RANGE, "range", offsetof(Openable, m_range));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_DESPAWN, "despawn", offsetof(Openable, m_despawn));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_INSTANT, "instant", offsetof(Openable, m_instant));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_KILL, "kill", offsetof(Openable, m_kill));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_OPENED, NULL, offsetof(Openable, m_opened));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_DURATION, "duration", offsetof(Openable, m_duration));
				aSchema->Define(ComponentSchema::TYPE_INT32, FIELD_DEAD_DESPAWN_TICKS, "dead_despawn_ticks", offsetof(Openable, m_deadDespawnTicks));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LEVEL, "level", offsetof(Openable, m_level));
				aSchema->DefineCustomObjects<Requirement>(FIELD_REQUIREMENTS, "requirements", offsetof(Openable, m_requirements));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_SOUND, "sound", offsetof(Openable, m_soundId))->SetDataType(DataType::ID_SOUND);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_COMPLETE_MANUAL_OBJECTIVE_ID, "complete_manual_objective", offsetof(Openable, m_completeManualObjectiveId))->SetDataType(DataType::ID_OBJECTIVE);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_TRIGGER_ABILITY_ID, "trigger_ability", offsetof(Openable, m_triggerAbilityId))->SetDataType(DataType::ID_ABILITY);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_REQUIRED_INCOMPLETE_QUEST_OBJECTIVE_ID, "required_incomplete_quest_objective", offsetof(Openable, m_requiredIncompleteQuestObjectiveId))->SetDataType(DataType::ID_OBJECTIVE);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_UNLOCK_MAP_TRIGGER_ID, "unlock_map_trigger", offsetof(Openable, m_unlockMapTriggerId))->SetDataType(DataType::ID_MAP_TRIGGER);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CONTEXT_HELP, "context_help", offsetof(Openable, m_contextHelpId))->SetDataType(DataType::ID_CONTEXT_HELP);
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_SPRITE_INDEX, "sprite_index", offsetof(Openable, m_spriteIndex));
				aSchema->DefineCustomObjects<Chat>(FIELD_CHATS, "chats", offsetof(Openable, m_chats));
			}

			void
			Reset()
			{
				m_lootTableId = 0;
				m_requiredItemId = 0;
				m_verb = Verb();
				m_requiredProfessionId = 0;
				m_requiredProfessionSkill = 0;
				m_requiredQuestId = 0;
				m_requiredIncompleteQuestObjectiveId = 0;
				m_range = 1;
				m_instant = false;
				m_despawn = false;
				m_opened = false;
				m_deadDespawnTicks = 0;
				m_kill = false;
				m_duration = 0;
				m_level = 1;
				m_requirements.clear();
				m_soundId = 0;
				m_completeManualObjectiveId = 0;
				m_triggerAbilityId = 0;
				m_unlockMapTriggerId = 0;
				m_contextHelpId = 0;
				m_spriteIndex = 0;
				m_chats.clear();
			}

			// Public data
			uint32_t					m_lootTableId = 0;
			uint32_t					m_requiredItemId = 0;
			Verb						m_verb;
			uint32_t					m_requiredProfessionId = 0;
			uint32_t					m_requiredProfessionSkill = 0;
			uint32_t					m_requiredQuestId = 0;
			uint32_t					m_requiredIncompleteQuestObjectiveId = 0;
			uint32_t					m_range = 1;
			bool						m_instant = false;
			bool						m_despawn = false;
			bool						m_opened = false;
			bool						m_kill = false;
			int32_t						m_deadDespawnTicks = 0;
			int32_t						m_duration = 0;
			uint32_t					m_level = 1;
			std::vector<Requirement>	m_requirements;
			uint32_t					m_soundId = 0;
			uint32_t					m_completeManualObjectiveId = 0;
			uint32_t					m_triggerAbilityId = 0;
			uint32_t					m_unlockMapTriggerId = 0;
			uint32_t					m_contextHelpId = 0;
			uint32_t					m_spriteIndex = 0;
			std::vector<Chat>			m_chats;
		};

	}

}