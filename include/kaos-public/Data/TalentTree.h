#pragma once

#include "../DataBase.h"

namespace kaos_public
{

	namespace Data
	{

		struct TalentTree
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_TALENT_TREE;

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