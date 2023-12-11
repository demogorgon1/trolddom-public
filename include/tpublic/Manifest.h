#pragma once

#include "Data/Ability.h"
#include "Data/Achievement.h"
#include "Data/AchievementCategory.h"
#include "Data/Aura.h"
#include "Data/Class.h"
#include "Data/CreatureType.h"
#include "Data/DialogueRoot.h"
#include "Data/DialogueScreen.h"
#include "Data/Entity.h"
#include "Data/Expression.h"
#include "Data/Faction.h"
#include "Data/Item.h"
#include "Data/LootGroup.h"
#include "Data/LootTable.h"
#include "Data/Map.h"
#include "Data/MapEntitySpawn.h"
#include "Data/MapPalette.h"
#include "Data/MapPlayerSpawn.h"
#include "Data/MapPortal.h"
#include "Data/MapSegment.h"
#include "Data/MapSegmentConnector.h"
#include "Data/MapTrigger.h"
#include "Data/NPCBehaviorState.h"
#include "Data/Objective.h"
#include "Data/ParticleSystem.h"
#include "Data/Profession.h"
#include "Data/Quest.h"
#include "Data/Sprite.h"
#include "Data/Talent.h"
#include "Data/TalentTree.h"
#include "Data/Zone.h"

#include "IReader.h"
#include "ItemMetrics.h"
#include "IWriter.h"
#include "NPCMetrics.h"
#include "PersistentIdTable.h"
#include "PlayerComponents.h"
#include "ProfessionMetrics.h"
#include "QuestMetrics.h"
#include "XPMetrics.h"

namespace tpublic
{

	class Manifest
	{
	public:
		struct IDataContainer						
		{
			// Virtual interface
			virtual void		Verify() const = 0;
			virtual void		PrepareRuntime(
									uint8_t				aRuntime,
									const Manifest*		aManifest) = 0;
			virtual void		ToStream(
									IWriter*			aStream) const = 0;
			virtual bool		FromStream(	
									IReader*			aStream) = 0;
			virtual DataBase*	GetBaseByName(
									PersistentIdTable*	aPersistentIdTable,
									const char*			aName) = 0;
		};

		template <typename _T>
		struct DataContainer
			: public IDataContainer
		{
			_T*
			GetByName(
				PersistentIdTable*						aPersistentIdTable,
				const char*								aName)
			{
				assert(!m_hasUnnamedEntries);

				typename std::unordered_map<std::string, _T*>::iterator it = m_nameTable.find(aName);
				if(it == m_nameTable.end())
				{
					if(aPersistentIdTable == NULL)
						return NULL;

					std::unique_ptr<_T> t = std::make_unique<_T>();
					t->m_name = aName;
					t->m_id = aPersistentIdTable->GetId(_T::DATA_TYPE, aName);

					if (t->m_id > m_maxId)
						m_maxId = t->m_id;

					m_nameTable.insert(std::pair<std::string, _T*>(t->m_name, t.get()));
					m_idTable.insert(std::pair<uint32_t, _T*>(t->m_id, t.get()));
					m_entries.push_back(std::move(t));

					return m_entries[m_entries.size() - 1].get();
				}

				return it->second;
			}

			_T*
			GetExistingByName(
				const char*								aName)
			{
				typename std::unordered_map<std::string, _T*>::iterator it = m_nameTable.find(aName);
				if(it == m_nameTable.end())
					return NULL;
				return it->second;
			}

			const _T*
			GetExistingByName(
				const char*								aName) const
			{
				typename std::unordered_map<std::string, _T*>::const_iterator it = m_nameTable.find(aName);
				if(it == m_nameTable.cend())
					return NULL;
				return it->second;
			}

			uint32_t
			GetExistingIdByName(
				const char*								aName) const
			{
				const _T* t = GetExistingByName(aName);
				TP_CHECK(t != NULL, "'%s' is not defined.", aName);
				return t->m_id;
			}

			_T*
			CreateUnnamed()
			{
				std::unique_ptr<_T> t = std::make_unique<_T>();
				t->m_id = ++m_maxId;
				m_idTable.insert(std::pair<uint32_t, _T*>(t->m_id, t.get()));
				_T* returnValue = t.get();
				m_entries.push_back(std::move(t));

				m_hasUnnamedEntries = true;
				return returnValue;
			}

			const _T*
			TryGetById(
				uint32_t								aId) const
			{
				auto it = m_idTable.find(aId);
				if(it == m_idTable.end())
					return NULL;
				return it->second;
			}

			_T*
			TryGetById(
				uint32_t								aId) 
			{
				auto it = m_idTable.find(aId);
				if(it == m_idTable.end())
					return NULL;
				return it->second;
			}

			const _T*
			GetById(
				uint32_t								aId) const
			{
				const _T* t = TryGetById(aId);
				TP_CHECK(t != NULL, "Invalid '%s' id: %u", DataType::IdToString(_T::DATA_TYPE), aId);
				return t;
			}

			_T*
			GetById(
				uint32_t								aId) 
			{
				_T* t = TryGetById(aId);
				TP_CHECK(t != NULL, "Invalid '%s' id: %u", DataType::IdToString(_T::DATA_TYPE), aId);
				return t;
			}

			void
			ForEach(
				std::function<void(_T*)>				aCallback)
			{
				for (std::unique_ptr<_T>& t : m_entries)
					aCallback(t.get());
			}

			void
			ForEach(
				std::function<void(const _T*)>			aCallback) const
			{
				for (const std::unique_ptr<_T>& t : m_entries)
					aCallback(t.get());
			}

			// IDataContainer implementation
			void
			Verify() const override
			{
				std::unordered_map<uint32_t, const _T*> idTable;

				for(const std::unique_ptr<_T>& t : m_entries)
				{
					t->Verify();

					TP_VERIFY(idTable.find(t->m_id) == idTable.end(), t->m_debugInfo, "Id collision: %u", t->m_id);
					idTable[t->m_id] = t.get();
				}
			}

			void		
			PrepareRuntime(
				uint8_t									aRuntime,
				const Manifest*							aManifest) override
			{
				for (const std::unique_ptr<_T>& t : m_entries)
					t->PrepareRuntime(aRuntime, aManifest);
			}

			void
			ToStream(
				IWriter*								aStream) const override
			{
				aStream->WriteObjectPointers(m_entries);
			}

			bool
			FromStream(
				IReader*								aStream) override
			{
				if(!aStream->ReadObjectPointers(m_entries))
					return false;

				for (std::unique_ptr<_T>& t : m_entries)
				{
					TP_CHECK(t->m_defined, "'%s' not defined.", DataType::IdToString(_T::DATA_TYPE));
					TP_CHECK(m_idTable.find(t->m_id) == m_idTable.end(), "Id collision for '%s': %u", DataType::IdToString(_T::DATA_TYPE), t->m_id);
					TP_CHECK(m_nameTable.find(t->m_name) == m_nameTable.end(), "Name collision for '%s': %s", DataType::IdToString(_T::DATA_TYPE), t->m_name.c_str());

					m_idTable[t->m_id] = t.get();
					m_nameTable[t->m_name] = t.get();

					if(t->m_id > m_maxId)
						m_maxId = t->m_id;
				}

				return true;
			}

			DataBase*
			GetBaseByName(
				PersistentIdTable*						aPersistentIdTable,
				const char*								aName)
			{
				return GetByName(aPersistentIdTable, aName);
			}

			// Public data
			std::vector<std::unique_ptr<_T>>			m_entries;
			std::unordered_map<std::string, _T*>		m_nameTable;
			std::unordered_map<uint32_t, _T*>			m_idTable;
			uint32_t									m_maxId = 0;
			bool										m_hasUnnamedEntries = false;
		};

		Manifest()
		{
			RegisterDataContainer(m_abilities);
			RegisterDataContainer(m_achievements);
			RegisterDataContainer(m_achievementCategories);
			RegisterDataContainer(m_auras);
			RegisterDataContainer(m_classes);
			RegisterDataContainer(m_creatureTypes);
			RegisterDataContainer(m_dialogueRoots);
			RegisterDataContainer(m_dialogueScreens);
			RegisterDataContainer(m_entities);
			RegisterDataContainer(m_expressions);
			RegisterDataContainer(m_factions);
			RegisterDataContainer(m_items);
			RegisterDataContainer(m_lootGroups);
			RegisterDataContainer(m_lootTables);
			RegisterDataContainer(m_maps);
			RegisterDataContainer(m_mapEntitySpawns);
			RegisterDataContainer(m_mapPalettes);
			RegisterDataContainer(m_mapPlayerSpawns);
			RegisterDataContainer(m_mapPortals);
			RegisterDataContainer(m_mapSegments);
			RegisterDataContainer(m_mapSegmentConnectors);
			RegisterDataContainer(m_mapTriggers);
			RegisterDataContainer(m_npcBehaviorStates);
			RegisterDataContainer(m_objectives);
			RegisterDataContainer(m_particleSystems);
			RegisterDataContainer(m_professions);
			RegisterDataContainer(m_quests);
			RegisterDataContainer(m_sprites);
			RegisterDataContainer(m_talents);
			RegisterDataContainer(m_talentTrees);
			RegisterDataContainer(m_zones);
		}
		
		~Manifest()
		{

		}
		
		void
		Verify() const
		{
			for(uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
			{
				assert(m_containers[i] != NULL);
				m_containers[i]->Verify();
			}
		}

		void
		ToStream(
			IWriter*									aStream) const
		{
			for (uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
			{
				assert(m_containers[i] != NULL);
				m_containers[i]->ToStream(aStream);
			}

			m_playerComponents.ToStream(aStream);
			m_xpMetrics.ToStream(aStream);
			m_itemMetrics.ToStream(aStream);
			m_npcMetrics.ToStream(aStream);
			m_questMetrics.ToStream(aStream);
			m_professionMetrics.ToStream(aStream);
		}
		
		bool
		FromStream(
			IReader*									aStream) 
		{
			for (uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
			{
				assert(m_containers[i] != NULL);
				if(!m_containers[i]->FromStream(aStream))
					return false;
			}

			if(!m_playerComponents.FromStream(aStream))
				return false;
			if (!m_xpMetrics.FromStream(aStream))
				return false;
			if (!m_itemMetrics.FromStream(aStream))
				return false;
			if (!m_npcMetrics.FromStream(aStream))
				return false;
			if (!m_questMetrics.FromStream(aStream))
				return false;
			if (!m_professionMetrics.FromStream(aStream))
				return false;

			return true;
		}

		void
		PrepareRuntime(
			uint8_t										aRuntime)
		{
			for (uint8_t i = 1; i < (uint8_t)DataType::NUM_IDS; i++)
			{
				assert(m_containers[i] != NULL);
				m_containers[i]->PrepareRuntime(aRuntime, this);
			}
		}

		// Public data
		DataContainer<Data::Ability>					m_abilities;
		DataContainer<Data::Achievement>				m_achievements;
		DataContainer<Data::AchievementCategory>		m_achievementCategories;
		DataContainer<Data::Aura>						m_auras;
		DataContainer<Data::Class>						m_classes;
		DataContainer<Data::CreatureType>				m_creatureTypes;
		DataContainer<Data::DialogueRoot>				m_dialogueRoots;
		DataContainer<Data::DialogueScreen>				m_dialogueScreens;
		DataContainer<Data::Entity>						m_entities;
		DataContainer<Data::Expression>					m_expressions;
		DataContainer<Data::Faction>					m_factions;
		DataContainer<Data::Item>						m_items;
		DataContainer<Data::LootGroup>					m_lootGroups;
		DataContainer<Data::LootTable>					m_lootTables;
		DataContainer<Data::Map>						m_maps;
		DataContainer<Data::MapEntitySpawn>				m_mapEntitySpawns;
		DataContainer<Data::MapPalette>					m_mapPalettes;
		DataContainer<Data::MapPlayerSpawn>				m_mapPlayerSpawns;
		DataContainer<Data::MapPortal>					m_mapPortals;
		DataContainer<Data::MapSegment>					m_mapSegments;
		DataContainer<Data::MapSegmentConnector>		m_mapSegmentConnectors;
		DataContainer<Data::MapTrigger>					m_mapTriggers;
		DataContainer<Data::NPCBehaviorState>			m_npcBehaviorStates;
		DataContainer<Data::Objective>					m_objectives;
		DataContainer<Data::ParticleSystem>				m_particleSystems;
		DataContainer<Data::Profession>					m_professions;
		DataContainer<Data::Quest>						m_quests;
		DataContainer<Data::Sprite>						m_sprites;
		DataContainer<Data::Talent>						m_talents;
		DataContainer<Data::TalentTree>					m_talentTrees;
		DataContainer<Data::Zone>						m_zones;

		IDataContainer*									m_containers[DataType::NUM_IDS] = { 0 };

		// Global non-itemized data
		PlayerComponents								m_playerComponents;
		XPMetrics										m_xpMetrics;
		ItemMetrics										m_itemMetrics;
		NPCMetrics										m_npcMetrics;
		QuestMetrics									m_questMetrics;
		ProfessionMetrics								m_professionMetrics;

	private:

		template <typename _T>
		void 
		RegisterDataContainer(
			DataContainer<_T>&							aContainer)
		{
			m_containers[_T::DATA_TYPE] = &aContainer;
		}

	};

}