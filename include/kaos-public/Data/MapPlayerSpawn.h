#pragma once

#include "../DataBase.h"

namespace kaos_public
{

	namespace Data
	{

		struct MapPlayerSpawn
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_PLAYER_SPAWN;

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		/*aNode*/) override
			{
			}

			// Public data
		};

	}

}