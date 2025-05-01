#pragma once

#include "ObjectiveTypes/CollectItems.h"
#include "ObjectiveTypes/DialogueTrigger.h"
#include "ObjectiveTypes/KillNPC.h"
#include "ObjectiveTypes/KillPlayer.h"
#include "ObjectiveTypes/ManualBoolean.h"
#include "ObjectiveTypes/ManualCounter.h"
#include "ObjectiveTypes/RouteNPC.h"
#include "ObjectiveTypes/UseAbility.h"

#include "ObjectiveTypeBase.h"

namespace tpublic
{

	class ObjectiveTypeFactory
	{
	public:
		ObjectiveTypeFactory()
		{
			_Register<ObjectiveTypes::CollectItems>();
			_Register<ObjectiveTypes::DialogueTrigger>();
			_Register<ObjectiveTypes::KillNPC>();
			_Register<ObjectiveTypes::KillPlayer>();
			_Register<ObjectiveTypes::ManualBoolean>();
			_Register<ObjectiveTypes::ManualCounter>();
			_Register<ObjectiveTypes::RouteNPC>();
			_Register<ObjectiveTypes::UseAbility>();
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
