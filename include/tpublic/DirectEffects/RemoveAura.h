#pragma once

#include <tpublic/Data/Aura.h>

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct RemoveAura
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_REMOVE_AURA;

			RemoveAura()
			{

			}

			virtual 
			~RemoveAura()
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
			Data::Aura::Type		m_auraType = Data::Aura::INVALID_TYPE;
			uint32_t				m_auraFlags = 0;
			uint32_t				m_auraId = 0;
			uint32_t				m_auraGroupId = 0;
		};

	}

}