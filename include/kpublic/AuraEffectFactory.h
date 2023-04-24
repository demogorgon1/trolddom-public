#pragma once

#include "AuraEffects/Stun.h"

#include "AuraEffect.h"

namespace kpublic
{

	class AuraEffectFactory
	{
	public:
		AuraEffectFactory()
		{
			_Register<AuraEffects::Stun>();
		}

		~AuraEffectFactory()
		{

		}

		AuraEffectBase*
		Create(
			uint32_t			aId) const
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<AuraEffectBase*()>			m_functions[AuraEffect::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}