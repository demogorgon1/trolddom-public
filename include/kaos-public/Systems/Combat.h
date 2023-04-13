#pragma once

#include "../SystemBase.h"

namespace kaos_public::Systems
{

	class Combat
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_COMBAT;

					Combat(
						const Manifest*	aManifest);
		virtual		~Combat();

		// SystemBase implementation
		void		Update(
						ComponentBase**	aComponents) override;

	};

}