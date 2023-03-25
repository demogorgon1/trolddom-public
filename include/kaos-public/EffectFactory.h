#pragma once

#include "Effects/Damage.h"
#include "Effects/Stun.h"

#include "Effect.h"

namespace kaos_public
{

	class EffectFactory
	{
	public:
		EffectFactory()
		{
			_Register<Effects::Damage>();
			_Register<Effects::Stun>();
		}

		~EffectFactory()
		{

		}

		EffectBase*
		Create(
			uint32_t			aId) 
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<EffectBase*()>			m_functions[Effect::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}