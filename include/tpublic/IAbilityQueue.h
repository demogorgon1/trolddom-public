#pragma once

#include <tpublic/Data/Ability.h>

namespace tpublic
{

	struct Vec2;

	class IAbilityQueue
	{
	public:
		virtual ~IAbilityQueue() {}

		enum AbilityQueueResult
		{
			ABILITY_QUEUE_RESULT_OK,
			ABILITY_QUEUE_RESULT_TOO_FAR_AWAY,
			ABILITY_QUEUE_RESULT_NO_LINE_OF_SIGHT,
			ABILITY_QUEUE_RESULT_FAILURE
		};

		// Virtual interface
		virtual AbilityQueueResult		AddAbility(
											uint32_t				aSourceEntityInstanceId,
											uint32_t				aTargetEntityInstanceId,
											const Vec2&				aAOETarget,
											const Data::Ability*	aAbility) = 0;
	};

}