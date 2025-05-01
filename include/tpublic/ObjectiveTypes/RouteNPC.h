#pragma once

#include "../ObjectiveTypeBase.h"

namespace tpublic::ObjectiveTypes
{

	class RouteNPC
		: public ObjectiveTypeBase
	{
	public:
		static const ObjectiveType::Id ID = ObjectiveType::ID_ROUTE_NPC;
		static const uint32_t FLAGS = FLAG_BOOLEAN;

								RouteNPC();
		virtual					~RouteNPC();

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
	};

}