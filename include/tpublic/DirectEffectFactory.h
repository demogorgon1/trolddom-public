#pragma once

#include "DirectEffect.h"

namespace tpublic
{

	class DirectEffectBase;

	class DirectEffectFactory
	{
	public:
								DirectEffectFactory();
								~DirectEffectFactory();
						
		DirectEffectBase*		Create(
									uint32_t			aId) const;
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