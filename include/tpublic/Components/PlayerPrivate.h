#pragma once

#include <tpublic/Data/Ability.h>

#include "../AbilityModifierList.h"
#include "../Component.h"
#include "../ComponentBase.h"
#include "../ErrorNotification.h"
#include "../EventHistory.h"
#include "../LootCooldowns.h"
#include "../PlayerGateways.h"
#include "../PlayerProfessions.h"
#include "../PlayerWorld.h"
#include "../PlayerWorship.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerPrivate
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_PRIVATE;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;
			
			struct GuildRegistrationHistory
			{
				GuildRegistrationHistory()
					: m_eventHistory(7 * 24 * 60 * 60) // Count guild registrations within last 7 days
				{

				}

				void
				ToStream(
					IWriter*	aWriter) const
				{
					m_eventHistory.ToStream(aWriter);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if(!m_eventHistory.FromStream(aReader))
						return false;
					return true;
				}

				// Public data
				EventHistory<3>		m_eventHistory;
			};

			enum Field
			{
				FIELD_LEVEL,
				FIELD_XP,
				FIELD_IS_DEAD,
				FIELD_RESURRECTION_POINT_POSITION,
				FIELD_RESURRECTION_POINT_MAP_ID,
				FIELD_GUILD_REGISTRATION_HISTORY,
				FIELD_PROFESSIONS,
				FIELD_WORSHIP,
				FIELD_GATEWAYS,
				DEPRECATED_FIELD_SELECTED_PLAYER_WORLD,
				FIELD_LATEST_CHARACTER_FIX_ID,
				FIELD_CLASS_VERSION,
				FIELD_KNOWS_RIDING,
				FIELD_LOOT_COOLDOWNS,
				FIELD_NON_HARDCORE_FLAG,
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LEVEL, NULL, offsetof(PlayerPrivate, m_level));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_XP, NULL, offsetof(PlayerPrivate, m_xp));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_IS_DEAD, NULL, offsetof(PlayerPrivate, m_isDead));
				aSchema->Define(ComponentSchema::TYPE_VEC2, FIELD_RESURRECTION_POINT_POSITION, NULL, offsetof(PlayerPrivate, m_resurrectionPointPosition));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_RESURRECTION_POINT_MAP_ID, NULL, offsetof(PlayerPrivate, m_resurrectionPointMapId));
				aSchema->DefineCustomObjectNoSource<GuildRegistrationHistory>(FIELD_GUILD_REGISTRATION_HISTORY, offsetof(PlayerPrivate, m_guildRegistrationHistory));
				aSchema->DefineCustomObjectNoSource<PlayerProfessions>(FIELD_PROFESSIONS, offsetof(PlayerPrivate, m_professions));
				aSchema->DefineCustomObjectNoSource<PlayerWorship>(FIELD_WORSHIP, offsetof(PlayerPrivate, m_worship));
				aSchema->DefineCustomObjectNoSource<PlayerGateways>(FIELD_GATEWAYS, offsetof(PlayerPrivate, m_gateways));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_LATEST_CHARACTER_FIX_ID, NULL, offsetof(PlayerPrivate, m_latestCharacterFixId));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_CLASS_VERSION, NULL, offsetof(PlayerPrivate, m_classVersion));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_KNOWS_RIDING, NULL, offsetof(PlayerPrivate, m_knowsRiding));
				aSchema->DefineCustomObjectNoSource<LootCooldowns>(FIELD_LOOT_COOLDOWNS, offsetof(PlayerPrivate, m_lootCooldowns));
				aSchema->Define(ComponentSchema::TYPE_BOOL, FIELD_NON_HARDCORE_FLAG, NULL, offsetof(PlayerPrivate, m_nonHardcoreFlag));

				aSchema->OnRead<PlayerPrivate>([](
					PlayerPrivate*				aPlayerPrivate,
					ComponentSchema::ReadType	aReadType,
					const Manifest*				aManifest)
				{
					if(aReadType == ComponentSchema::READ_TYPE_STORAGE)
						aPlayerPrivate->OnLoadedFromPersistence(aManifest);
				});				
			}

			void
			Reset()
			{
				m_level = 1;
				m_xp = 0;
				m_isDead = false;
				m_resurrectionPointPosition = Vec2();
				m_resurrectionPointMapId = 0;
				m_guildRegistrationHistory = GuildRegistrationHistory();
				m_professions.m_entries.clear();
				m_worship.m_table.clear();
				m_gateways.m_currentSeed = 0;
				m_gateways.m_lockedSeeds.clear();
				m_latestCharacterFixId = 0;
				m_classVersion = 0;
				m_knowsRiding = false;
				m_lootCooldowns.Reset();
				m_nonHardcoreFlag = false;

				m_tryEditPlayerWorlds = false;
				m_recall = false;
				m_xpGain = 0;
				m_professionSkillUseEvents.clear();
				m_errorNotification = ErrorNotification::INVALID_ID;
				m_abilityModifierList = NULL;
				m_equippedItemTypeFlags = 0;
				m_useAbilityFlags = 0;
				m_useAbilityExtendedFlags = 0;
				m_notifyLearnedRiding = false;
				m_entityCompassEvent.reset();
				m_pendingMapTransfer.reset();
				m_mapId = 0;
			}

			bool
			HasUseAbility() const
			{
				return m_useAbilityExtendedFlags != 0 || m_useAbilityFlags != 0;
			}

			void
			OnLoadedFromPersistence(
				const Manifest*					aManifest)
			{
				m_professions.OnLoadedFromPersistence(aManifest);
			}

			// Public data
			uint32_t														m_level = 1;
			uint32_t														m_xp = 0;
			bool															m_isDead = false;
			Vec2															m_resurrectionPointPosition;
			uint32_t														m_resurrectionPointMapId = 0;
			GuildRegistrationHistory										m_guildRegistrationHistory;
			PlayerProfessions												m_professions;
			PlayerWorship													m_worship;
			PlayerGateways													m_gateways;
			uint32_t														m_latestCharacterFixId = 0;
			uint32_t														m_classVersion = 0;
			bool															m_knowsRiding = false;
			LootCooldowns													m_lootCooldowns;
			bool															m_nonHardcoreFlag = false;

			// Not serialized, internal
			uint32_t														m_mapId = 0;
			bool															m_tryEditPlayerWorlds = false;
			bool															m_recall = false;
			uint32_t														m_xpGain = 0;
			bool															m_professionsUpdated = false;
			bool															m_notifyLearnedRiding = false;

			struct ProfessionSkillUseEvent
			{
				uint32_t													m_professionId = 0;
				uint32_t													m_professionSkill = 0;
			};

			std::vector<ProfessionSkillUseEvent>							m_professionSkillUseEvents;			
			ErrorNotification::Id											m_errorNotification = ErrorNotification::INVALID_ID;
			std::optional<Vec2>												m_entityCompassEvent;

			const tpublic::AbilityModifierList*								m_abilityModifierList = NULL;

			uint16_t														m_equippedItemTypeFlags = 0;
			uint32_t														m_useAbilityFlags = 0;
			uint32_t														m_useAbilityExtendedFlags = 0;

			std::vector<uint32_t>											m_incrementCharacterStatIds;

			struct PendingMapTransfer
			{
				uint32_t													m_mapId = 0;
				uint32_t													m_mapPlayerSpawnId = 0;
			};

			std::optional<PendingMapTransfer>								m_pendingMapTransfer;
		};
	}

}