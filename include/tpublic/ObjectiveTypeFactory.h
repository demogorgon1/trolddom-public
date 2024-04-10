#pragma once

#include "ObjectiveTypes/CollectItems.h"
#include "ObjectiveTypes/KillNPC.h"

#include "ObjectiveTypeBase.h"

namespace tpublic
{

	class ObjectiveTypeFactory
	{
	public:
		ObjectiveTypeFactory()
		{
			_Register<ObjectiveTypes::CollectItems>();
			_Register<ObjectiveTypes::KillNPC>();
		}

		~ObjectiveTypeFactory()
		{

		}

		ObjectiveTypeBase*
		Create(
			uint32_t			aId) const
		{
			if(!m_functions[aId])
				return NULL;

			return m_functions[aId]();
		}

	private:

		std::function<ObjectiveTypeBase*()>			m_functions[ObjectiveType::NUM_IDS];

		template<typename _T>
		void
		_Register()
		{
			m_functions[_T::ID] = []() { return new _T(); };
		}

	};

}
