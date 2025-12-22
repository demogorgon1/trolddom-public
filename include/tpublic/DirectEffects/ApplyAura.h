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

			enum SourceRedirect : uint8_t
			{
				SOURCE_REDIRECT_NONE,
				SOURCE_REDIRECT_TARGET_OF_TARGET,
				SOURCE_REDIRECT_TARGET,
				SOURCE_REDIRECT_REFRESH
			};

			static SourceRedirect
			SourceToSourceRedirect(
				const SourceNode*								aSource) 
			{
				if(aSource->IsIdentifier("none"))
					return SOURCE_REDIRECT_NONE;
				else if (aSource->IsIdentifier("target_of_target"))
					return SOURCE_REDIRECT_TARGET_OF_TARGET;
				else if (aSource->IsIdentifier("target"))
					return SOURCE_REDIRECT_TARGET;
				else if (aSource->IsIdentifier("refresh"))
					return SOURCE_REDIRECT_REFRESH;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid source redirect.", aSource->GetIdentifier());
				return SOURCE_REDIRECT_NONE;
			}

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
			std::vector<uint32_t>	m_auraIds;
			int32_t					m_threat = 0;			
			uint32_t				m_applyToPartyMembersInRange = 0;
			SourceRedirect			m_sourceRedirect = SOURCE_REDIRECT_NONE;		
		};

	}

}