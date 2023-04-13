#pragma once

#include "../SystemBase.h"

namespace kaos_public::Systems
{

	class NPC
		: public SystemBase
	{
	public:
		static const System::Id ID = System::ID_NPC;

					NPC(
						const Manifest*		aManifest);
		virtual		~NPC();

		// SystemBase implementation
		void		Init(
						ComponentBase**		aComponents) override;
		void		Update(
						ComponentBase**		aComponents) override;

	private:
	};

}