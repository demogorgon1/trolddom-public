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
				const SourceNode*		aNode) override
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
				uint8_t					aRuntime,
				const Manifest*			aManifest) override
			{
				assert(m_data);
				m_data->PrepareRuntime(aRuntime, aManifest);
			}

			// Public data
			std::unique_ptr<MapData>	m_data;
		};

	}

}