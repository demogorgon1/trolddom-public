#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct ResetCooldowns
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_RESET_COOLDOWNS;

			ResetCooldowns()
			{

			}

			virtual 
			~ResetCooldowns()
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
								const IWorldView*				aWorldView,
								bool							aOffHand) override;

			// Public data
			std::vector<uint32_t>	m_talentTreeIds;
		};

	}

}