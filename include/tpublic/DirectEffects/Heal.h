#pragma once

#include "../CombatFunction.h"
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
								IResourceChangeQueue*			aResourceChangeQueue,
								IAuraEventQueue*				aAuraEventQueue,
								IEventQueue*					aEventQueue,
								const IWorldView*				aWorldView) override;
			bool			CalculateToolTipHeal(
								const EntityInstance*			aEntityInstance,
								UIntRange&						aOutHeal) const override;

			// Public data
			CombatFunction		m_function;
			bool				m_maxHealthPercentage = false;
		};

	}

}