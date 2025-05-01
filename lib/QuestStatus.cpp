#include "Pcheader.h"

#include <tpublic/Components/ActiveQuests.h>
#include <tpublic/Components/CombatPublic.h>
#include <tpublic/Components/CompletedQuests.h>

#include <tpublic/Data/Quest.h>

#include <tpublic/EntityInstance.h>
#include <tpublic/Manifest.h>
#include <tpublic/QuestStatus.h>

namespace tpublic
{

	namespace QuestStatus
	{

		Id	
		Determine(
			const Manifest*				aManifest,
			const EntityInstance*		aPlayerEntity,
			uint32_t					aQuestId)
		{
			const Components::CompletedQuests* completedQuests = aPlayerEntity->GetComponent<Components::CompletedQuests>();
			if (completedQuests->HasQuest(aQuestId))
				return ID_COMPLETED;
				
			const Components::ActiveQuests* activeQuests = aPlayerEntity->GetComponent<Components::ActiveQuests>();
			if(activeQuests->HasQuest(aQuestId))
				return ID_IN_PROGRESS;

			const Components::CombatPublic* combatPublic = aPlayerEntity->GetComponent<Components::CombatPublic>();
			const Data::Quest* quest = aManifest->GetById<Data::Quest>(aQuestId);
			if(quest->m_level > combatPublic->m_level && quest->m_level - combatPublic->m_level >= 10)
				return ID_PLAYER_TOO_LOW_LEVEL;

			for(uint32_t prerequisiteQuestId : quest->m_prerequisites)
			{
				if(!completedQuests->HasQuest(prerequisiteQuestId))
					return ID_UNAVAILABLE;
			}

			return ID_AVAILABLE;
		}

		void
		GetColor(
			const Manifest*				aManifest,
			const EntityInstance*		aPlayerEntity,
			uint32_t					aQuestId,
			uint8_t						aOutColor[3])
		{
			const Components::CombatPublic* combatPublic = aPlayerEntity->GetComponent<Components::CombatPublic>();
			const Data::Quest* quest = aManifest->GetById<Data::Quest>(aQuestId);
			int32_t levelDiff = (int32_t)quest->m_level - (int32_t)combatPublic->m_level;
			aManifest->m_xpMetrics.GetLevelDiffColor(levelDiff, aOutColor);
		}
	}

}