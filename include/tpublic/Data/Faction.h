#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct Faction
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_FACTION;
			static const bool TAGGED = true;

			enum Flag : uint8_t
			{
				FLAG_NEUTRAL = 0x01,
				FLAG_FRIENDLY = 0x02,
				FLAG_REPUTATION = 0x04
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
						if (aChild->m_name == "flags")
						{
							aChild->GetArray()->ForEachChild([&](
								const SourceNode* aFlag)
							{
								if (aFlag->IsIdentifier("neutral"))
									m_flags |= FLAG_NEUTRAL;
								else if (aFlag->IsIdentifier("friendly"))
									m_flags |= FLAG_FRIENDLY;
								else if (aFlag->IsIdentifier("reputation"))
									m_flags |= FLAG_REPUTATION;
								else
									TP_VERIFY(false, aFlag->m_debugInfo, "'%s' is not a valid faction flag.", aFlag->m_value.c_str());
							});
						}
						else if(aChild->m_name == "string")
						{
							m_string = aChild->GetString();
						}
						else
						{
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
						}
					}
				});
				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WritePOD(m_flags);
				aStream->WriteString(m_string);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadPOD(m_flags))
					return false;
				if(!aStream->ReadString(m_string))
					return false;
				return true;
			}

			// Helpers
			bool IsNeutralOrFriendly() const { return (m_flags & FLAG_NEUTRAL) != 0 || (m_flags & FLAG_FRIENDLY) != 0; }

			// Public data
			uint8_t		m_flags;
			std::string	m_string;
		};

	}

}