#pragma once

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
					TYPE_PICK_UP
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
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_OPENED, NULL, offsetof(Openable, m_opened));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_DURATION, "duration", offsetof(Openable, m_duration));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LEVEL, "level", offsetof(Openable, m_level));
				aSchema->DefineCustomObjects<Requirement>(FIELD_REQUIREMENTS, "requirements", offsetof(Openable, m_requirements));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_SOUND, "sound", offsetof(Openable, m_soundId))->SetDataType(DataType::ID_SOUND);
			}

			// Public data
			uint32_t					m_lootTableId = 0;
			uint32_t					m_requiredItemId = 0;
			Verb						m_verb;
			uint32_t					m_requiredProfessionId = 0;
			uint32_t					m_requiredProfessionSkill = 0;
			uint32_t					m_requiredQuestId = 0;
			uint32_t					m_range = 1;
			bool						m_instant = false;
			bool						m_despawn = false;
			bool						m_opened = false;
			uint32_t					m_duration = 0;
			uint32_t					m_level = 1;
			std::vector<Requirement>	m_requirements;
			uint32_t					m_soundId = 0;
		};

	}

}