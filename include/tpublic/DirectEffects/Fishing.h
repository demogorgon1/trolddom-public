#pragma once

#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct Fishing
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_FISHING;

			Fishing()
			{

			}

			virtual 
			~Fishing()
			{

			}

			// DirectEffectBase implementation
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
						IResourceChangeQueue*			aCombatResultQueue,
						IAuraEventQueue*				aAuraEventQueue,
						IEventQueue*					aEventQueue,
						const IWorldView*				aWorldView) override;

			// Public data
		};

	}

}