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
				SPAWN_FLAG_AT_TARGET		= 0x01,
				SPAWN_FLAG_NO_OWNER			= 0x02,
				SPAWN_FLAG_SOURCE_LEVEL		= 0x04,
				SPAWN_FLAG_AT_AOE_TARGET	= 0x08
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
					else if (strcmp(string, "at_aoe_target") == 0)
						flags |= SPAWN_FLAG_AT_AOE_TARGET;
					else
						TP_VERIFY(false, aFlag->m_debugInfo, "'%s' is not a valid spawn flag.", string);
				});
				return flags;
			}

			struct MustHaveOneAtTarget
			{
				MustHaveOneAtTarget()
				{

				}

				MustHaveOneAtTarget(
					const SourceNode*							aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "creature_types")
							aChild->GetIdArray(DataType::ID_CREATURE_TYPE, m_creatureTypeIds);
						else if (aChild->m_name == "entities")
							aChild->GetIdArray(DataType::ID_ENTITY, m_entityIds);
						else if (aChild->m_name == "entity_states")
							aChild->GetIdArrayWithLookup<EntityState::Id, EntityState::INVALID_ID>(m_entityStateIds, [](const char* aString) { return EntityState::StringToId(aString); });
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void 
				ToStream(
					IWriter*									aWriter) const
				{
					aWriter->WriteUInts(m_creatureTypeIds);
					aWriter->WritePODs(m_entityStateIds);
					aWriter->WriteUInts(m_entityIds);
				}

				bool
				FromStream(
					IReader*									aReader) 
				{
					if(!aReader->ReadUInts(m_creatureTypeIds))
						return false;
					if(!aReader->ReadPODs(m_entityStateIds))
						return false;
					if (!aReader->ReadUInts(m_entityIds))
						return false;
					return true;
				}

				// Public data
				std::vector<uint32_t>			m_creatureTypeIds;
				std::vector<EntityState::Id>	m_entityStateIds;
				std::vector<uint32_t>			m_entityIds;
			};

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
			std::vector<MustHaveOneAtTarget>	m_mustHaveOneAtTarget;
		};

	}

}