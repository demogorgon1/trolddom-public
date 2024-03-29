#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Zone
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ZONE;
			static const bool TAGGED = true;

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
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				aWriter->WriteString(m_string);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadString(m_string))
					return false;
				return true;
			}

			// Public data
			std::string			m_string;
		};

	}

}