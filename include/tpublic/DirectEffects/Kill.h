#pragma once

#include "../Components/CombatPublic.h"

#include "../DirectEffectBase.h"
#include "../EntityInstance.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct Kill
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_KILL;

			Kill()
			{

			}

			virtual 
			~Kill()
			{

			}

			// DirectEffectBase implementation
			void
			FromSource(
				const Parser::Node*			/*aSource*/) override
			{
			}

			void
			ToStream(
				IWriter*					aStream) const override
			{
				ToStreamBase(aStream);
			}

			bool
			FromStream(
				IReader*					aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				return true;
			}

			void
			Resolve(
				uint32_t					aTick,
				std::mt19937&				/*aRandom*/,
				const Manifest*				/*aManifest*/,
				CombatEvent::Id				/*aId*/,
				uint32_t					/*aAbilityId*/,
				const EntityInstance*		/*aSource*/,
				EntityInstance*				aTarget,
				IResourceChangeQueue*		aCombatResultQueue,
				IAuraEventQueue*			/*aAuraEventQueue*/,
				IThreatEventQueue*			/*aThreatEventQueue*/) override
			{
				aCombatResultQueue->AddUpdateCallback([aTarget, aTick]()
				{
					aTarget->SetState(EntityState::ID_DEAD, aTick);
				});
			}

			// Public data
		};

	}

}