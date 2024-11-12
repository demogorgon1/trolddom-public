#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct TriggerAbility
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_TRIGGER_ABILITY;

			enum Target : uint8_t
			{
				INVALID_TARGET,

				TARGET_RANDOM_NEARBY
			};

			static Target
			SourceToTarget(
				const SourceNode*								aSource)
			{
				if(aSource->IsIdentifier("random_nearby"))
					return TARGET_RANDOM_NEARBY;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid target.", aSource->GetIdentifier());
				return INVALID_TARGET;
			}


			TriggerAbility()
			{

			}

			virtual 
			~TriggerAbility()
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
			Target		m_target = INVALID_TARGET;
			uint32_t	m_abilityId = 0;
		};

	}

}