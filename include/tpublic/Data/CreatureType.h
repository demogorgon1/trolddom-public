#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct CreatureType
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_CREATURE_TYPE;

			enum Flag : uint8_t
			{
				FLAG_CASH_LOOT = 0x01
			}; 

			static uint8_t
			StringToFlag(
				const char*				aString)
			{
				std::string_view t(aString);
				if(t == "cash_loot")
					return FLAG_CASH_LOOT;
				return 0;
			}

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
					const SourceNode*	aChild)
				{
					if(aChild->m_name == "string")
					{
						m_string = aChild->GetString();
					}
					else if (aChild->m_name == "flags")
					{
						aChild->GetArray()->ForEachChild([&](
							const SourceNode* aFlag)
						{
							uint8_t flag = StringToFlag(aFlag->GetIdentifier());
							TP_VERIFY(flag != 0, aFlag->m_debugInfo, "'%s' is not a valid flag.", aFlag->GetIdentifier());
							m_flags |= flag;
						});								
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteString(m_string);
				aStream->WritePOD(m_flags);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadPOD(m_flags))
					return false;
				return true;
			}

			// Public data
			std::string				m_string;
			uint8_t					m_flags;
		};


	}

}