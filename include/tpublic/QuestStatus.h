#pragma once

namespace tpublic
{

	class EntityInstance;
	class Manifest;

	namespace QuestStatus
	{

		enum Id
		{
			ID_PLAYER_TOO_LOW_LEVEL,
			ID_UNAVAILABLE,
			ID_COMPLETED,
			ID_IN_PROGRESS,
			ID_AVAILABLE
		};

		Id		Determine(
					const Manifest*						aManifest, 
					const EntityInstance*				aPlayerEntity,
					uint32_t							aQuestId);
		void	GetColor(
					const Manifest*						aManifest, 
					const EntityInstance*				aPlayerEntity,
					uint32_t							aQuestId,
					uint8_t								aOutColor[3]);

	}

}