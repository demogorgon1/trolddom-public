#pragma once

#include "../DataBase.h"
#include "../ProfessionType.h"

namespace tpublic
{

	namespace Data
	{

		struct Profession
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_PROFESSION;

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
				TP_VERIFY(aNode->m_annotation, aNode->m_debugInfo, "Missing type annotation.");
				m_type = ProfessionType::StringToId(aNode->m_annotation->GetIdentifier());
				TP_VERIFY(m_type != ProfessionType::INVALID_ID, aNode->m_debugInfo, "'%s' is not a valid profession type.", aNode->m_annotation->GetIdentifier());

				aNode->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "string")
						m_string = aChild->GetString();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);

				aStream->WriteString(m_string);
				aStream->WritePOD(m_type);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;

				if(!aStream->ReadString(m_string))
					return false;
				if(!aStream->ReadPOD(m_type))
					return false;
				if(!ProfessionType::Validate(m_type))
					return false;
				return true;
			}

			// Public data
			std::string				m_string;
			ProfessionType::Id		m_type;
		};

	}

}