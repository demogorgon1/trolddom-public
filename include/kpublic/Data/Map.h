#pragma once

#include "../DataBase.h"
#include "../MapData.h"

namespace tpublic
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

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				assert(m_data);
				m_data->ToStream(aStream);
			}
			
			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				m_data = std::make_unique<MapData>();
				return m_data->FromStream(aStream);
			}

			void
			PrepareRuntime(
				const Manifest*			aManifest) override
			{
				assert(m_data);
				m_data->PrepareRuntime(aManifest);
			}

			// Public data
			std::unique_ptr<MapData>	m_data;
		};

	}

}