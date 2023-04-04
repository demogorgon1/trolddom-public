#pragma once

#include "../DataBase.h"
#include "../MapData.h"

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
				const Parser::Node*		aNode) override
			{
				m_data = std::make_unique<MapData>(aNode);
			}

			// Public data
			std::unique_ptr<MapData>	m_data;
		};

	}

}