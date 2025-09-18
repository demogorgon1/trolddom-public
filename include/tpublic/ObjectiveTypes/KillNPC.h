#pragma once

#include "../ObjectiveTypeBase.h"

namespace tpublic::ObjectiveTypes
{

	class KillNPC
		: public ObjectiveTypeBase
	{
	public:
		static const ObjectiveType::Id ID = ObjectiveType::ID_KILL_NPC;
		static const uint32_t FLAGS = 0;

								KillNPC();
		virtual					~KillNPC();

		// ObjectiveTypeBase implementation
		void					FromSource(
									const SourceNode*		aSource) override;
		void					ToStream(
									IWriter*				aWriter) override;
		bool					FromStream(
									IReader*				aReader) override;
		ObjectiveInstanceBase*	CreateInstance() const override;
		void					GetWatchedEntities(
									std::vector<uint32_t>&	aOutEntityIds) const override;
		
	private:

		class Instance;

		std::vector<uint32_t>	m_entityIds;
		uint32_t				m_count = 1;
		bool					m_boolean = false;
	};

}