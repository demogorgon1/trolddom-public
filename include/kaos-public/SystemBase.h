#pragma once

#include "Component.h"
#include "System.h"

namespace kaos_public
{
	
	class EntityInstance;
	class Manifest;

	class SystemBase
	{
	public:
		SystemBase(
			const Manifest*										aManifest)
			: m_manifest(aManifest)
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
											ComponentBase**		/*aComponents*/) { }
		virtual void					Update(
											ComponentBase**		/*aComponents*/) { }

		// Data access
		const Manifest*					GetManifest() const { return m_manifest; }
		const std::vector<uint32_t>&	GetRequiredComponents() const { return m_requiredComponents; }

	private:

		const Manifest*			m_manifest;
		std::vector<uint32_t>	m_requiredComponents;
		size_t					m_componentIndices[Component::NUM_IDS];
	};

}