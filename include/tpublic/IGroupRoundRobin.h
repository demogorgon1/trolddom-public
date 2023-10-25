#pragma once

namespace tpublic
{

	class IGroupRoundRobin
	{
	public:
		virtual				~IGroupRoundRobin() {}

		// Virtual interface
		virtual uint32_t	GetNextGroupRoundRobinEntityInstanceId(
								uint64_t				aGroupId) = 0;
	};

}