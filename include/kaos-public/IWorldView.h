#pragma once

namespace kaos_public
{

	class IWorldView
	{
	public:
		typedef std::function<bool(const EntityInstance*)> EntityCallback;

		virtual				~IWorldView() {}	

		// Virtual interface
		virtual size_t		QueryAllEntityInstances(
								EntityCallback			aEntityCallback) const = 0;
											
	};

}