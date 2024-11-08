#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct SpawnEntity
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_SPAWN_ENTITY;

			enum SpawnFlag : uint8_t
			{
				SPAWN_FLAG_AT_TARGET	= 0x01,
				SPAWN_FLAG_NO_OWNER		= 0x02,
				SPAWN_FLAG_SOURCE_LEVEL	= 0x04
			};

			static uint8_t
			SourceToSpawnFlags(
				const SourceNode* aSource)
			{
				uint8_t flags = 0;
				aSource->GetArray()->ForEachChild([&](
					const SourceNode* aFlag)
				{
					const char* string = aFlag->GetIdentifier();
					if (strcmp(string, "at_target") == 0)
						flags |= SPAWN_FLAG_AT_TARGET;
					else if (strcmp(string, "no_owner") == 0)
						flags |= SPAWN_FLAG_NO_OWNER;
					else if (strcmp(string, "source_level") == 0)
						flags |= SPAWN_FLAG_SOURCE_LEVEL;
					else
						TP_VERIFY(false, aFlag->m_debugInfo, "'%s' is not a valid spawn flag.", string);
				});
				return flags;
			}

			struct RefreshNPCMetrics
			{
				RefreshNPCMetrics()
				{

				}

				RefreshNPCMetrics(
					const SourceNode*							aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "weapon_damage")
							m_weaponDamage = aChild->GetFloat();
						else if (aChild->m_name == "health")
							m_health = aChild->GetFloat();
						else if (aChild->m_name == "armor")
							m_armor = aChild->GetFloat();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*									aWriter) const
				{
					aWriter->WriteFloat(m_weaponDamage);
					aWriter->WriteFloat(m_health);
					aWriter->WriteFloat(m_armor);
				}

				bool
				FromStream(
					IReader*									aReader)
				{
					if (!aReader->ReadFloat(m_weaponDamage))
						return false;
					if (!aReader->ReadFloat(m_health))
						return false;
					if (!aReader->ReadFloat(m_armor))
						return false;
					return true;
				}
				
				// Public data
				float			m_weaponDamage = 1.0f;
				float			m_health = 1.0f;
				float			m_armor = 1.0f;
			};

			SpawnEntity()
			{

			}

			virtual 
			~SpawnEntity()
			{

			}

			// DirectEffectBase implementation
			void			FromSource(
								const SourceNode*				aSource) override;
			void			ToStream(
								IWriter*						aStream) const override;
			bool			FromStream(
								IReader*						aStream) override;
			Result			Resolve(
								int32_t							aTick,
								std::mt19937&					aRandom,
								const Manifest*					aManifest,
								CombatEvent::Id					aId,
								uint32_t						aAbilityId,
								const SourceEntityInstance&		aSourceEntityInstance,
								EntityInstance*					aSource,
								EntityInstance*					aTarget,
								const Vec2&						aAOETarget,
								const ItemInstanceReference&	aItem,
								IResourceChangeQueue*			aCombatResultQueue,
								IAuraEventQueue*				aAuraEventQueue,
								IEventQueue*					aEventQueue,
								const IWorldView*				aWorldView) override;

			// Public data
			EntityState::Id						m_initState = EntityState::ID_DEFAULT;
			uint32_t							m_mapEntitySpawnId = 0;
			uint32_t							m_entityId = 0;
			uint8_t								m_spawnFlags = 0;
			int32_t								m_npcTargetThreat = 0;
			std::optional<RefreshNPCMetrics>	m_refreshNPCMetrics;
		};

	}

}