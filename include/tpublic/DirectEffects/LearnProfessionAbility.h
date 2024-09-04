#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct LearnProfessionAbility
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_LEARN_PROFESSION_ABILITY;

			LearnProfessionAbility()
			{

			}

			virtual 
			~LearnProfessionAbility()
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
			uint32_t		GetToolTipItemId(
								const Manifest*					aManifest) const override;

			// Public data
			uint32_t			m_professionId = 0;
			uint32_t			m_abilityId = 0;
		};

	}

}