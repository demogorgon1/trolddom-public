#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct ProduceItem
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_PRODUCE_ITEM;

			ProduceItem()
			{

			}

			virtual 
			~ProduceItem()
			{

			}

			// DirectEffectBase implementation
			void
			FromSource(
				const SourceNode*			aSource) override;
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
			uint32_t			m_itemId = 0;
		};

	}

}