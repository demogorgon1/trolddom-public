#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct LootGroup
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_LOOT_GROUP;
			static const bool TAGGED = true;

			struct Oracle
			{
				Oracle()
				{

				}

				Oracle(
					const SourceNode*	aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "source_plural")
							m_sourcePlural = aChild->GetString();
						else if(aChild->m_name == "everywhere")
							m_everywhere = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteString(m_sourcePlural);
					aStream->WriteBool(m_everywhere);
				}

				bool
				FromStream(
					IReader*				aStream) 
				{
					if(!aStream->ReadString(m_sourcePlural))
						return false;
					if(!aStream->ReadBool(m_everywhere))
						return false;
					return true;
				}

				// Public data
				std::string		m_sourcePlural;
				bool			m_everywhere = false;
			};

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
				aNode->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_name == "oracle")
							m_oracle = std::make_unique<Oracle>(aChild);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteOptionalObjectPointer(m_oracle);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadOptionalObjectPointer(m_oracle))
					return false;
				return true;
			}

			// Public data
			std::unique_ptr<Oracle>	m_oracle;
		};

	}

}