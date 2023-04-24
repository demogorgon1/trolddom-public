#pragma once

#include "../Components/CombatPublic.h"

#include "../DirectEffectBase.h"
#include "../EntityInstance.h"

namespace kpublic
{

	namespace Effects
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

			// EffectBase implementation
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
				std::mt19937&				aRandom,
				Components::CombatPublic*	aSourceCombatPublic,
				Components::CombatPrivate*	aSourceCombatPrivate,
				Components::CombatPublic*	aTargetCombatPublic,
				CombatEvent::Id				aId,
				uint32_t					aAbilityId,
				uint32_t					aSourceEntityInstanceId,
				uint32_t					aTargetEntityInstanceId,
				IResourceChangeQueue*			aCombatResultQueue)
			{
				(void)aRandom;
				(void)aSourceCombatPublic;
				(void)aSourceCombatPrivate;
				(void)aTargetCombatPublic;
				(void)aId;
				(void)aCombatResultQueue;
				(void)aAbilityId;
				(void)aSourceEntityInstanceId;
				(void)aTargetEntityInstanceId;
			}

			// Public data
		};

	}

}