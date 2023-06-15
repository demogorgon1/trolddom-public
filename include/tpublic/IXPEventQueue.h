#pragma once

#include "PlayerTag.h"

namespace tpublic
{

	class IXPEventQueue
	{
	public:
		virtual				~IXPEventQueue() {}

		// Virtual interface
		virtual void		AddKillXPEvent(
								const PlayerTag&		aPlayerTag,
								uint32_t				aKillLevel) = 0;
	};

}