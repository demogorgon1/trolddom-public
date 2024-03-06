#pragma once

#include "../DirectEffectBase.h"
#include "../UIntCurve.h"

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
						const SourceNode*				aSource) override;
			void	ToStream(
						IWriter*						aStream) const override;
			bool	FromStream(
						IReader*						aStream) override;
			void	Resolve(
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
			DirectEffect::DamageType	m_damageType = DirectEffect::DAMAGE_TYPE_PHYSICAL;
			DirectEffect::DamageBase	m_damageBase = DirectEffect::DAMAGE_BASE_RANGE;

			uint32_t					m_damageBaseRangeMin = 0;
			uint32_t					m_damageBaseRangeMax = 0;

			UIntCurve<uint32_t>			m_levelCurve;
		};

	}

}