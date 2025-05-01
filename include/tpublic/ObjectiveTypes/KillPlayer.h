#pragma once

#include "../ObjectiveTypeBase.h"

namespace tpublic::ObjectiveTypes
{

	class KillPlayer
		: public ObjectiveTypeBase
	{
	public:
		static const ObjectiveType::Id ID = ObjectiveType::ID_KILL_PLAYER;
		static const uint32_t FLAGS = 0;

								KillPlayer();
		virtual					~KillPlayer();

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

		uint32_t				m_count = 1;
		uint32_t				m_factionId = 0;
	};

}