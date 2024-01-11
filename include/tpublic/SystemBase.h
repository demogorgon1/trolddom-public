#pragma once

#include "Component.h"
#include "ComponentBase.h"
#include "EntityState.h"
#include "System.h"

namespace tpublic
{
	
	class EntityInstance;
	class IEventQueue;
	class IWorldView;
	class LootGenerator;
	class Manifest;
	class Pantheons;

	class SystemBase
	{
	public:
		struct Context
		{
			// Public data
			IEventQueue*			m_eventQueue = NULL;
			const IWorldView*		m_worldView = NULL;
			int32_t					m_tick = 0;
			std::mt19937*			m_random = NULL;
		};

		struct SystemData
		{
			const LootGenerator*	m_lootGenerator = NULL;
			const Pantheons*		m_pantheons = NULL;
			const Manifest*			m_manifest = NULL;
			uint32_t				m_mapId = 0;
			uint32_t				m_mapSeed = 0;
		};

		SystemBase(
			const SystemData*									aData,
			int32_t												aUpdateInterval = 0)
			: m_data(aData)
			, m_updateInterval(aUpdateInterval)
		{
			memset(m_componentIndices, 0xFF, sizeof(m_componentIndices));
		}

		virtual			
		~SystemBase() 
		{

		}

		template <typename _T>
		void
		RequireComponent()
		{
			m_componentIndices[_T::ID] = m_requiredComponents.size();
			m_requiredComponents.push_back(_T::ID);
		}

		template <typename _T>
		_T*
		GetComponent(
			ComponentBase**										aComponents)
		{
			size_t i = m_componentIndices[_T::ID];
			assert(i != UINT64_MAX);
			return aComponents[i]->Cast<_T>();
		}

		// Virtual methods
		virtual void					Init(
											uint32_t			/*aEntityId*/,
											uint32_t			/*aEntityInstanceId*/,
											EntityState::Id		/*aEntityState*/,
											ComponentBase**		/*aComponents*/,
											int32_t				/*aTick*/) { }
		virtual EntityState::Id			UpdatePrivate(
											uint32_t			/*aEntityId*/,
											uint32_t			/*aEntityInstanceId*/,
											EntityState::Id		/*aEntityState*/,
											int32_t				/*aTicksInState*/,
											ComponentBase**		/*aComponents*/,
											Context*			/*aContext*/) { return EntityState::CONTINUE; }
		virtual void					UpdatePublic(
											uint32_t			/*aEntityId*/,
											uint32_t			/*aEntityInstanceId*/,
											EntityState::Id		/*aEntityState*/,
											int32_t				/*aTicksInState*/,
											ComponentBase**		/*aComponents*/,
											Context*			/*aContext*/) { }

		// Data access
		const Manifest*					GetManifest() const { return m_data->m_manifest; }
		const SystemData*				GetData() const { return m_data; }
		const std::vector<uint32_t>&	GetRequiredComponents() const { return m_requiredComponents; }
		int32_t							GetUpdateInterval() const { return m_updateInterval; }

	private:

		const SystemData*		m_data;
		std::vector<uint32_t>	m_requiredComponents;
		size_t					m_componentIndices[Component::NUM_IDS];
		int32_t					m_updateInterval;
	};

}