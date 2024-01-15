#pragma once

#include "DirectEffects/ApplyAura.h"
#include "DirectEffects/Damage.h"
#include "DirectEffects/Fishing.h"
#include "DirectEffects/Heal.h"
#include "DirectEffects/Simple.h"
#include "DirectEffects/Threat.h"

#include "DirectEffect.h"

namespace tpublic
{

	class DirectEffectFactory
	{
	public:
		DirectEffectFactory()
		{
			_Register<DirectEffects::ApplyAura>();
			_Register<DirectEffects::Damage>();
			_Register<DirectEffects::Fishing>();
			_Register<DirectEffects::Heal>();
			_Register<DirectEffects::Simple>();
			_Register<DirectEffects::Threat>();
		}

		~DirectEffectFactory()
		{

		}

		DirectEffectBase*
		Create(
			uint32_t			aId) const
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<DirectEffectBase*()>			m_functions[DirectEffect::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}