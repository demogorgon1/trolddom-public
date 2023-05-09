#pragma once

namespace kpublic
{

	class IThreatEventQueue
	{
	public:
		virtual				~IThreatEventQueue() {}

		// Virtual interface
		virtual void		AddThreatEvent(
								uint32_t				aSourceEntityInstanceId,
								uint32_t				aTargetEntityInstanceId,
								int32_t					aThreat) = 0;
		virtual void		AddThreatClearEvent(
								uint32_t				aEntityInstanceId) = 0;
								
	};

}