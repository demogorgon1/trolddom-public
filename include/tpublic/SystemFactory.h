#pragma once

#include "Systems/Combat.h"
#include "Systems/Environment.h"
#include "Systems/NPC.h"

#include "System.h"

namespace tpublic
{

	class Manifest;

	class SystemFactory
	{
	public:
		SystemFactory(
			const Manifest*	aManifest)
			: m_manifest(aManifest)
		{
			_Register<Systems::Combat>();
			_Register<Systems::Environment>();
			_Register<Systems::NPC>();
		}

		~SystemFactory()
		{

		}

		SystemBase*
		Create(
			uint32_t		aId) 
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<SystemBase*()>	m_functions[System::NUM_IDS];
		const Manifest*					m_manifest;

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = [&]() { return new _T(m_manifest); };
		}

	};

}