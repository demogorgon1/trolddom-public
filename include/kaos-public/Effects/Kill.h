#pragma once

#include "../Components/CombatPublic.h"

#include "../EffectBase.h"
#include "../EntityInstance.h"

namespace kaos_public
{

	namespace Effects
	{

		struct Kill
			: public EffectBase
		{
			static const Effect::Id ID = Effect::ID_KILL;

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
				ICombatResultQueue*			aCombatResultQueue)
			{
				(void)aRandom;
				(void)aSourceCombatPublic;
				(void)aSourceCombatPrivate;
				(void)aTargetCombatPublic;
				(void)aId;
				(void)aCombatResultQueue;
			}

			// Public data
		};

	}

}