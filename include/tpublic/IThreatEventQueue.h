#pragma once

namespace tpublic
{

	class IThreatEventQueue
	{
	public:
		virtual				~IThreatEventQueue() {}

		// Virtual interface
		virtual void		AddThreatEvent(
								uint32_t					aSourceEntityInstanceId,
								uint32_t					aTargetEntityInstanceId,
								int32_t						aThreat,
								const std::optional<float>&	aMultiply = std::optional<float>()) = 0;
		virtual void		AddThreatClearEvent(
								uint32_t					aEntityInstanceId) = 0;
								
	};

}