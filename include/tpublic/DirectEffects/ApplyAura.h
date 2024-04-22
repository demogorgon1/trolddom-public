#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct ApplyAura
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_APPLY_AURA;

			ApplyAura()
			{

			}

			virtual 
			~ApplyAura()
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
								IResourceChangeQueue*			aCombatResultQueue,
								IAuraEventQueue*				aAuraEventQueue,
								IEventQueue*					aEventQueue,
								const IWorldView*				aWorldView) override;

			// Public data
			uint32_t				m_auraId = 0;
			int32_t					m_threat = 0;			
			uint32_t				m_applyToPartyMembersInRange = 0;
			bool					m_targetSelf = false;
		};

	}

}