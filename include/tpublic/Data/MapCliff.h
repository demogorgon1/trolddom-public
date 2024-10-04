#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct MapCliff
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_CLIFF;
			static const bool TAGGED = false;			

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if (!FromSourceBase(aChild))
					{
						if (aChild->m_name == "cliff_style")
							m_cliffStyleId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_CLIFF_STYLE, aChild->GetIdentifier());
						else if(aChild->m_name == "elevation")
							m_elevation = aChild->GetUInt8();
						else if (aChild->m_name == "ramp")
							m_ramp = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_cliffStyleId);
				aStream->WritePOD(m_elevation);
				aStream->WritePOD(m_ramp);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_cliffStyleId))
					return false;
				if (!aStream->ReadPOD(m_elevation))
					return false;
				if (!aStream->ReadPOD(m_ramp))
					return false;
				return true;
			}

			// Public data
			uint32_t			m_cliffStyleId = 0;
			uint8_t				m_elevation = 0;
			bool				m_ramp = false;
		};

	}

}