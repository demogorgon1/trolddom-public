#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct Heal
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_HEAL;

			Heal()
			{

			}

			virtual 
			~Heal()
			{

			}

			// EffectBase implementation
			void			FromSource(
								const SourceNode*				aSource) override;
			void			ToStream(
								IWriter*						aStream) const override;
			bool			FromStream(
								IReader*						aStream) override;
			CombatEvent::Id	Resolve(
								int32_t							aTick,
								std::mt19937&					aRandom,
								const Manifest*					aManifest,
								CombatEvent::Id					aId,
								uint32_t						aAbilityId,
								EntityInstance*					aSource,
								EntityInstance*					aTarget,
								const ItemInstanceReference&	aItem,
								IResourceChangeQueue*			aResourceChangeQueue,
								IAuraEventQueue*				aAuraEventQueue,
								IEventQueue*					aEventQueue,
								const IWorldView*				aWorldView) override;

			// Public data
			uint32_t			m_baseMin = 0;
			uint32_t			m_baseMax = 0;
		};

	}

}