#pragma once

#include "AuraEffect.h"

namespace tpublic
{

	class AuraEffectBase;

	class AuraEffectFactory
	{
	public:
							AuraEffectFactory();
							~AuraEffectFactory();

		AuraEffectBase*		Create(
								uint32_t			aId) const;

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
