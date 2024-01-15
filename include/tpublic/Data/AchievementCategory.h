#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct AchievementCategory
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ACHIEVEMENT_CATEGORY;
			static const bool TAGGED = false;

			void
			Verify() const
			{
				VerifyBase();

				TP_VERIFY(m_orderKey != 0 && m_orderKey != UINT32_MAX, m_debugInfo, "Invalid order key.");
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
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if (aChild->m_name == "order_key")
							m_orderKey = aChild->GetUInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteString(m_string);
				aStream->WriteUInt(m_orderKey);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadString(m_string))
					return false;
				if(!aStream->ReadUInt(m_orderKey))
					return false;
				return true;
			}

			// Public data
			std::string		m_string;
			uint32_t		m_orderKey = 1;
		};

	}

}