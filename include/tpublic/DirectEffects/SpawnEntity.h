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
				SPAWN_FLAG_NO_OWNER		= 0x02
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
					else
						TP_VERIFY(false, aFlag->m_debugInfo, "'%s' is not a valid spawn flag.", string);
				});
				return flags;
			}

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
								EntityInstance*					aSource,
								EntityInstance*					aTarget,
								const Vec2&						aAOETarget,
								const ItemInstanceReference&	aItem,
								IResourceChangeQueue*			aCombatResultQueue,
								IAuraEventQueue*				aAuraEventQueue,
								IEventQueue*					aEventQueue,
								const IWorldView*				aWorldView) override;

			// Public data
			uint32_t				m_mapEntitySpawnId = 0;
			uint32_t				m_entityId = 0;
			uint8_t					m_spawnFlags = 0;
			int32_t					m_npcTargetThreat = 0;
		};

	}

}