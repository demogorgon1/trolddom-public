#pragma once

#include "Systems/Combat.h"
#include "Systems/Environment.h"
#include "Systems/Door.h"
#include "Systems/Gateway.h"
#include "Systems/HealthRegen.h"
#include "Systems/ManaRegen.h"
#include "Systems/Minion.h"
#include "Systems/NPC.h"
#include "Systems/Openable.h"
#include "Systems/PVPRift.h"
#include "Systems/Rage.h"
#include "Systems/RandomItemVendor.h"
#include "Systems/Shrine.h"
#include "Systems/Trigger.h"
#include "Systems/VendorRestock.h"

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
			_Register<Systems::Door>();
			_Register<Systems::Environment>();
			_Register<Systems::Gateway>();
			_Register<Systems::HealthRegen>();
			_Register<Systems::ManaRegen>();
			_Register<Systems::Minion>();
			_Register<Systems::NPC>();
			_Register<Systems::Openable>();
			_Register<Systems::PVPRift>();
			_Register<Systems::Rage>();
			_Register<Systems::RandomItemVendor>();
			_Register<Systems::Shrine>();
			_Register<Systems::Trigger>();
			_Register<Systems::VendorRestock>();
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

