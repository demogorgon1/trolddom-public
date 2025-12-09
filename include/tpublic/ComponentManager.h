#pragma once

#include "Component.h"
#include "ComponentBase.h"
#include "ComponentPool.h"
#include "ComponentSchema.h"

namespace tpublic
{

	class IReader;
	class IWriter;
	class Manifest;
	class UniqueComponent;

	class ComponentManager
	{
	public:
									ComponentManager();
									~ComponentManager();

		void						AllocateUniqueComponent(
										uint32_t				aId,
										UniqueComponent&		aOut) const;
		ComponentBase*				AllocateComponent(
										uint32_t				aId);
		void						ReleaseComponent(
										ComponentBase*			aComponent);
		const ComponentSchema*		GetComponentSchema(
										uint32_t				aId) const;
		uint8_t						GetComponentFlags(
										uint32_t				aId) const;
		Persistence::Id				GetComponentPersistence(
										uint32_t				aId) const;
		ComponentBase::Replication	GetComponentReplication(
										uint32_t				aId) const;
		std::string					AsDebugString(
										const ComponentBase*	aComponent) const;
		void						WriteNetwork(
										IWriter*				aWriter,
										const ComponentBase*	aComponent) const;
		bool						ReadNetwork(
										IReader*				aReader,
										ComponentBase*			aComponent) const;
		void						WriteStorage(
										IWriter*				aWriter,
										const ComponentBase*	aComponent) const;
		bool						ReadStorage(
										IReader*				aReader,
										ComponentBase*			aComponent,
										const Manifest*			aManifest) const;
		void						ReadSource(
										const SourceNode*		aSource,
										ComponentBase*			aComponent) const;

	private:

		struct ComponentType
		{
			uint32_t							m_id = 0;
			std::function<ComponentBase*()>		m_create;
			std::function<void(ComponentBase*)>	m_delete;
			ComponentSchema						m_schema;
			uint8_t								m_flags = 0;
			Persistence::Id						m_persistence = Persistence::ID_NONE;
			ComponentBase::Replication			m_replication = ComponentBase::REPLICATION_NONE;
			std::unique_ptr<ComponentPoolBase>	m_pool;
		};

		ComponentType							m_componentTypes[Component::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			ComponentType& t = m_componentTypes[_T::ID];

			_T::CreateSchema(&t.m_schema);

			t.m_id = _T::ID;
			t.m_flags = _T::FLAGS;
			t.m_persistence = _T::PERSISTENCE;
			t.m_replication = _T::REPLICATION;
			t.m_pool.reset(new ComponentPool<_T>());

			t.m_create = []() { return new _T(); };
			t.m_delete = [](ComponentBase* aComponentBase) { delete aComponentBase->Cast<_T>(); };

			_InitComponentType(t);
		}

		void				_InitComponentType(
								ComponentType&		aComponentType);
	};

}