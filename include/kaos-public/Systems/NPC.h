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
		virtual			~NPC();

		// SystemBase implementation
		void			Init(
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							ComponentBase**		aComponents) override;
		EntityState::Id	Update(
							uint32_t			aEntityInstanceId,
							EntityState::Id		aEntityState,
							ComponentBase**		aComponents,
							Context*			aContext) override;

	private:
	};

}