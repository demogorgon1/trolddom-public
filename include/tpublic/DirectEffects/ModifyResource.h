#pragma once

#include "../CombatFunction.h"
#include "../DirectEffectBase.h"
#include "../IntRange.h"
#include "../Resource.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct ModifyResource
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_MODIFY_RESOURCE;

			ModifyResource()
			{

			}

			virtual 
			~ModifyResource()
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

			// Public data
			Resource::Id	m_resourceId = Resource::INVALID_ID;
			bool			m_targetSelf = false;
			CombatFunction	m_function;
		};

	}

}