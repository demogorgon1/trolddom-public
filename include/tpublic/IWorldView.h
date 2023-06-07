#pragma once

namespace tpublic
{

	class IWorldView
	{
	public:
		typedef std::function<bool(const EntityInstance*)> EntityCallback;

		virtual							~IWorldView() {}	

		// Virtual interface
		virtual size_t					QueryAllEntityInstances(
											EntityCallback			aEntityCallback) const = 0;
		virtual const EntityInstance*	QuerySingleEntityInstance(
											uint32_t				aEntityInstanceId) const = 0;
		virtual size_t					QueryEntityInstancesAtPosition(
											const tpublic::Vec2&	aPosition,
											EntityCallback			aEntityCallback) const = 0;
											
	};

}