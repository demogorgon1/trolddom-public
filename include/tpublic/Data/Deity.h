#pragma once

#include "../DataBase.h"
#include "../Stat.h"

namespace tpublic
{

	namespace Data
	{

		struct Deity
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_DEITY;
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
						else if (aChild->m_name == "title_prefix")
							m_titlePrefix = aChild->GetString();
						else if (aChild->m_name == "title_suffix")
							m_titleSuffix = aChild->GetString();
						else if (aChild->m_name == "title")
							m_title = aChild->GetString();
						else if (aChild->m_name == "pantheon")
							m_pantheonId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PANTHEON, aChild->GetIdentifier());
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				ToStreamBase(aWriter);

				aWriter->WriteString(m_string);
				aWriter->WriteString(m_titlePrefix);
				aWriter->WriteString(m_titleSuffix);
				aWriter->WriteString(m_title);
				aWriter->WriteUInt(m_pantheonId);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if (!FromStreamBase(aReader))
					return false;

				if(!aReader->ReadString(m_string))
					return false;
				if (!aReader->ReadString(m_titlePrefix))
					return false;
				if (!aReader->ReadString(m_titleSuffix))
					return false;
				if (!aReader->ReadString(m_title))
					return false;
				if(!aReader->ReadUInt(m_pantheonId))
					return false;
				return true;
			}

			// Public data
			std::string							m_string;
			std::string							m_titlePrefix;
			std::string							m_titleSuffix;
			std::string							m_title;
			uint32_t							m_pantheonId = 0;
		};

	}

}