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
						else if(aChild->m_name == "town")
							m_town = aChild->GetBool();
						else if (aChild->m_name == "no_map")
							m_noMap = aChild->GetBool();
						else if (aChild->m_name == "can_query_position")
							m_canQueryPosition = aChild->GetBool();
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
				aWriter->WriteBool(m_town);
				aWriter->WriteBool(m_noMap);
				aWriter->WriteBool(m_canQueryPosition);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if(!aReader->ReadString(m_string))
					return false;
				if(!aReader->ReadBool(m_town))
					return false;
				if (!aReader->ReadBool(m_noMap))
					return false;
				if (!aReader->ReadBool(m_canQueryPosition))
					return false;
				return true;
			}

			void
			ToPropertyTable(
				PropertyTable&			aOut) const override
			{
				ToPropertyTableBase(aOut);
				aOut["string"] = m_string;
			}

			// Public data
			std::string			m_string;
			bool				m_town = false;			
			bool				m_noMap = false;
			bool				m_canQueryPosition = false;
		};

	}

}