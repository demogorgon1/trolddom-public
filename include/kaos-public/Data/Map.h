#pragma once

#include "../DataBase.h"

namespace kaos_public
{

	namespace Data
	{

		struct Map
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP;

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
				printf("..\n");
			}

			// Public data
		};

	}

}