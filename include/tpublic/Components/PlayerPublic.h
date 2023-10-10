#pragma once

#include "../Component.h"
#include "../Stat.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerPublic
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_PUBLIC;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS | FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			enum Field
			{
				FIELD_CLASS_ID,
				FIELD_CHARACTER_ID,
				FIELD_STATS,
				FIELD_GUILD_NAME
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CLASS_ID, NULL, offsetof(PlayerPublic, m_classId));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CHARACTER_ID, NULL, offsetof(PlayerPublic, m_characterId));
				aSchema->DefineCustomObjectNoSource<Stat::Collection>(FIELD_STATS, offsetof(PlayerPublic, m_stats));
				aSchema->Define(ComponentSchema::TYPE_STRING, FIELD_GUILD_NAME, NULL, offsetof(PlayerPublic, m_guildName));
			}

			PlayerPublic()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual
			~PlayerPublic()
			{

			}

			// ComponentBase implementation
			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInt(m_classId);
				aStream->WriteUInt(m_characterId);
				m_stats.ToStream(aStream);
				aStream->WriteString(m_guildName);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInt(m_classId))
					return false;
				if (!aStream->ReadUInt(m_characterId))
					return false;
				if(!m_stats.FromStream(aStream))
					return false;
				if(!aStream->ReadString(m_guildName))
					return false;
				return true;
			}

			void
			DebugPrint() const override
			{
				printf("player_public: class=%u guild_name=%s", m_classId, m_guildName.c_str());
				m_stats.DebugPrint();
				printf("\n");				
			}

			// Public data
			uint32_t			m_classId = 0;
			uint32_t			m_characterId = 0;
			Stat::Collection	m_stats;
			std::string			m_guildName;
		};
	}

}