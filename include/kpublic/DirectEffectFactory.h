#pragma once

#include "Effects/Damage.h"
#include "Effects/Heal.h"
#include "Effects/Kill.h"
#include "Effects/Stun.h"

#include "DirectEffect.h"

namespace kpublic
{

	class DirectEffectFactory
	{
	public:
		DirectEffectFactory()
		{
			_Register<Effects::Damage>();
			_Register<Effects::Heal>();
			_Register<Effects::Kill>();
			_Register<Effects::Stun>();
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