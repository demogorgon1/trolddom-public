#pragma once

#include "../DataBase.h"
#include "../Vec2.h"

namespace tpublic
{

	namespace Data
	{

		struct MapSegmentConnector
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_SEGMENT_CONNECTOR;
			static const bool TAGGED = false;

			void
			Verify() const
			{
				VerifyBase();
			}

			bool
			ConnectsWith(
				uint32_t				aMapSegmentConnectorId) const
			{
				for(uint32_t t : m_connects)
				{
					if(t == aMapSegmentConnectorId)
						return true;
				}
				return false;
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{	
				aNode->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "connects")
							aChild->GetIdArray(DataType::ID_MAP_SEGMENT_CONNECTOR, m_connects);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInts(m_connects);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadUInts(m_connects))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>	m_connects;
		};

	}

}