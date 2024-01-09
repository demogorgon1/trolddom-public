#pragma once

#include "Systems/Combat.h"
#include "Systems/Environment.h"
#include "Systems/HealthRegen.h"
#include "Systems/ManaRegen.h"
#include "Systems/NPC.h"
#include "Systems/Openable.h"
#include "Systems/Shrine.h"

#include "System.h"

namespace tpublic
{

	class Manifest;

	class SystemFactory
	{
	public:
		SystemFactory(
			const SystemBase::SystemData*	aData)
			: m_data(aData)
		{
			_Register<Systems::Combat>();
			_Register<Systems::Environment>(); 
			_Register<Systems::HealthRegen>();
			_Register<Systems::ManaRegen>();
			_Register<Systems::NPC>();
			_Register<Systems::Openable>();
			_Register<Systems::Shrine>();
		}

		~SystemFactory()
		{

		}

		SystemBase*
		Create(
			uint32_t						aId) 
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<SystemBase*()>	m_functions[System::NUM_IDS];
		const SystemBase::SystemData*	m_data;

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = [&]() { return new _T(m_data); };
		}

	};

}

