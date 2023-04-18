#pragma once

namespace kpublic
{

	namespace Components
	{
		struct CombatPublic;
	}

	class ICombatResultQueue
	{
	public:
		virtual				~ICombatResultQueue() {}

		// Virtual interface
		virtual void		AddResourceChange(
								Components::CombatPublic*		aCombat,
								size_t							aResourceIndex,
								int32_t							aChange) = 0;
	};

}