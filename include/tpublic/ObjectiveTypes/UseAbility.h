#pragma once

#include "../ObjectiveTypeBase.h"

namespace tpublic::ObjectiveTypes
{

	class UseAbility
		: public ObjectiveTypeBase
	{
	public:
		static const ObjectiveType::Id ID = ObjectiveType::ID_USE_ABILITY;
		static const uint32_t FLAGS = FLAG_UNMANAGED;

								UseAbility();
		virtual					~UseAbility();

		// ObjectiveTypeBase implementation
		void					FromSource(
									const SourceNode*		aSource) override;
		void					ToStream(
									IWriter*				aWriter) override;
		bool					FromStream(
									IReader*				aReader) override;
		ObjectiveInstanceBase*	CreateInstance() const override;
		
	private:

		class Instance;

		std::vector<uint32_t>	m_entityIds;
		uint32_t				m_abilityId = 0;
		uint32_t				m_count = 1;
		bool					m_boolean = false;
	};

}