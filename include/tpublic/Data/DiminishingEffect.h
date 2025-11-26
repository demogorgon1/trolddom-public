#pragma once

#include "../DataBase.h"
#include "../FloatCurve.h"
#include "../Requirement.h"

namespace tpublic
{

	namespace Data
	{

		struct DiminishingEffect
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_DIMINISHING_EFFECT;
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
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "curve")
							m_curve = FloatCurve(aChild);
						else if(aChild->m_name == "ticks")
							m_ticks = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				m_curve.ToStream(aWriter);
				aWriter->WriteInt(m_ticks);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!m_curve.FromStream(aReader))
					return false;
				if(!aReader->ReadInt(m_ticks))
					return false;
				return true;
			}

			// Public data
			FloatCurve			m_curve;
			int32_t				m_ticks = 0;
		};

	}

}