#pragma once

#include <tpublic/Data/Entity.h>

namespace tpublic
{
	
	namespace Components
	{
		struct CombatPublic;
		struct CombatPrivate;
		struct MinionPrivate;
		struct NPC;
	}

	class NPCMetrics;

	namespace ApplyNPCMetrics
	{

		void		Process(
						const NPCMetrics*				aNPCMetrics,
						const Data::Entity::Modifiers&	aModifiers,
						Components::CombatPublic*		aCombatPublic,
						Components::CombatPrivate*		aCombatPrivate,
						Components::MinionPrivate*		aMinionPrivate,
						Components::NPC*				aNPC,
						bool							aUpdate,
						bool							aBaseOnNPCResource);
		void		MakeEliteEasy(
						const NPCMetrics*				aNPCMetrics,
						Components::CombatPublic*		aCombatPublic,
						Components::CombatPrivate*		aCombatPrivate);
	}

}