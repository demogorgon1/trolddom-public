#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct Push
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_PUSH;

			Push()
			{

			}

			virtual 
			~Push()
			{

			}

			// DirectEffectBase implementation
			void
			FromSource(
				const Parser::Node*			aSource) override;
			void
			ToStream(
				IWriter*					aStream) const override;
			bool
			FromStream(
				IReader*					aStream) override;
			void
			Resolve(
				int32_t						aTick,
				std::mt19937&				aRandom,
				const Manifest*				aManifest,
				CombatEvent::Id				aId,
				uint32_t					aAbilityId,
				const EntityInstance*		aSource,
				EntityInstance*				aTarget,
				IResourceChangeQueue*		aCombatResultQueue,
				IAuraEventQueue*			aAuraEventQueue,
				IThreatEventQueue*			aThreatEventQueue) override;

			// Public data
		};

	}

}