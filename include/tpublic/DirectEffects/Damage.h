#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct Damage
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_DAMAGE;

			Damage()
			{

			}

			virtual 
			~Damage()
			{

			}

			// EffectBase implementation
			void	FromSource(
						const Parser::Node*		aSource) override;
			void	ToStream(
						IWriter*				aStream) const override;
			bool	FromStream(
						IReader*				aStream) override;
			void	Resolve(
						int32_t					aTick,
						std::mt19937&			aRandom,
						const Manifest*			aManifest,
						CombatEvent::Id			aId,
						uint32_t				aAbilityId,
						const EntityInstance*	aSource,
						EntityInstance*			aTarget,
						IResourceChangeQueue*	aResourceChangeQueue,
						IAuraEventQueue*		aAuraEventQueue,
						IThreatEventQueue*		aThreatEventQueue) override;

			// Public data
			DirectEffect::DamageType	m_damageType = DirectEffect::DAMAGE_TYPE_PHYSICAL;
			DirectEffect::DamageBase	m_damageBase = DirectEffect::DAMAGE_BASE_RANGE;

			uint32_t					m_damageBaseRangeMin = 0;
			uint32_t					m_damageBaseRangeMax = 0;
		};

	}

}