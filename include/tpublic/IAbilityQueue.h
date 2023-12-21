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
			ABILITY_QUEUE_RESULT_MISSING_INGREDIENTS,
			ABILITY_QUEUE_RESULT_NOT_ENOUGH_SPACE_IN_INVENTORY,
			ABILITY_QUEUE_RESULT_FAILURE
		};

		struct ProfessionSkillUpEvent
		{
			uint32_t		m_professionId = 0;
			uint32_t		m_skill = 0;
		};

		// Virtual interface
		virtual AbilityQueueResult		AddAbility(
											uint32_t				aSourceEntityInstanceId,
											uint32_t				aTargetEntityInstanceId,
											const Vec2&				aAOETarget,
											const Data::Ability*	aAbility,
											ProfessionSkillUpEvent*	aOutProfessionSkillUpEvent) = 0;
	};

}