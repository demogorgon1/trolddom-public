#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Emote
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_EMOTE;
			static const bool TAGGED = false;

			enum String : uint8_t
			{				
				STRING_SELF_TO_EVERYONE,
				STRING_OTHER_TO_EVERYONE,
				STRING_SELF_TO_TARGET,
				STRING_OTHER_TO_TARGET,

				NUM_STRINGS,

				INVALID_STRING
			};

			void
			Verify() const
			{
				VerifyBase();

				TP_VERIFY(m_strings.size() == NUM_STRINGS, m_debugInfo, "Invalid number of strings.");
			}

			const char*
			GetString(
				String					aString) const
			{
				assert((size_t)aString < (size_t)NUM_STRINGS);
				return m_strings[aString].c_str();
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
						if (aChild->m_name == "strings")
							aChild->GetStringArray(m_strings);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteStrings(m_strings);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadStrings(m_strings))
					return false;
				return true;
			}

			// Public data
			std::vector<std::string>	m_strings;
		};

	}

}