#include "Pcheader.h"

#include <tpublic/Components/CombatPublic.h>

#include <tpublic/Data/DialogueRoot.h>
#include <tpublic/Data/DialogueScreen.h>
#include <tpublic/Data/Entity.h>
#include <tpublic/Data/Map.h>
#include <tpublic/Data/MapEntitySpawn.h>

#include <tpublic/Manifest.h>

namespace tpublic
{
	
	namespace
	{
		void 
		_GetPossibleQuests(
			const Manifest*					aManifest, 
			uint32_t						aDialogueScreenId,
			std::unordered_set<uint32_t>&	aVisited,
			std::vector<uint32_t>&			aOutQuestIds)
		{
			aVisited.insert(aDialogueScreenId);

			const Data::DialogueScreen* dialogueScreen = aManifest->GetById<Data::DialogueScreen>(aDialogueScreenId);
			for(const Data::DialogueScreen::Option& option : dialogueScreen->m_options)
			{
				if(option.m_questId != 0 && !option.m_noPointOfInterest && option.m_questCompletion) // FIXME: only quest completion for now
					aOutQuestIds.push_back(option.m_questId);
				else if(option.m_dialogueScreenId != 0 && !aVisited.contains(option.m_dialogueScreenId))
					_GetPossibleQuests(aManifest, option.m_dialogueScreenId, aVisited, aOutQuestIds);
			}
		}
			
		void
		_EntityAtPosition(
			const Manifest*					aManifest,
			const Vec2&						aPosition,
			uint32_t						aEntityId,
			Data::Map*						aMap)
		{
			const Data::Entity* entity = aManifest->GetById<Data::Entity>(aEntityId);
			const Components::CombatPublic* combatPublic = entity->TryGetComponent<Components::CombatPublic>();
			if(combatPublic != NULL && combatPublic->m_dialogueRootId != 0)
			{
				const Data::DialogueRoot* dialogueRoot = aManifest->GetById<Data::DialogueRoot>(combatPublic->m_dialogueRootId);

				std::vector<uint32_t> possibleQuestIds;
				std::unordered_set<uint32_t> visited;

				for (const std::unique_ptr<Data::DialogueRoot::Entry>& entry : dialogueRoot->m_entries)
					_GetPossibleQuests(aManifest, entry->m_dialogueScreenId, visited, possibleQuestIds);

				if(possibleQuestIds.size() > 0)
				{
					MapData::PointOfInterest t;
					t.m_type = MapData::POINT_OF_INTEREST_TYPE_QUEST_GIVER;
					t.m_position = aPosition;
					t.m_questIds = std::move(possibleQuestIds);
					t.m_dialogueRootId = combatPublic->m_dialogueRootId;
					t.m_entityId = aEntityId;
					aMap->m_data->m_pointsOfInterest.push_back(t);
				}
			}
		}
	}

	namespace PostProcessMaps
	{

		void		
		Run(
			Manifest*						aManifest)
		{
			aManifest->GetContainer<Data::Map>()->ForEach([&](
				Data::Map* aMap)
			{
				for(const MapData::EntitySpawn& entitySpawn : aMap->m_data->m_entitySpawns)
				{
					if(entitySpawn.m_entityId != 0)
					{
						_EntityAtPosition(aManifest, { entitySpawn.m_x, entitySpawn.m_y }, entitySpawn.m_entityId, aMap);
					}
					else
					{
						const Data::MapEntitySpawn* mapEntitySpawn = aManifest->GetById<Data::MapEntitySpawn>(entitySpawn.m_mapEntitySpawnId);
						for(const std::unique_ptr<Data::MapEntitySpawn::Entity>& entity : mapEntitySpawn ->m_entities)
							_EntityAtPosition(aManifest, { entitySpawn.m_x, entitySpawn.m_y }, entity->m_entityId, aMap);
					}
				}
				return true;
			});			
		}

	}

}