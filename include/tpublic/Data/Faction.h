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

			enum Flag : uint8_t
			{
				FLAG_NEUTRAL = 0x01
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aNode) override
			{	
				aNode->ForEachChild([&](
					const Parser::Node* aChild)
				{
					if(aChild->m_name == "flags")
					{
						aChild->GetArray()->ForEachChild([&](
							const Parser::Node* aFlag)
						{
							if(aFlag->m_name == "neutral")
								m_flags |= FLAG_NEUTRAL;
							else
								TP_VERIFY(false, aFlag->m_debugInfo, "'%s' is not a valid faction flag.", aFlag->m_name.c_str());
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
				aStream->WritePOD(m_flags);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadPOD(m_flags))
					return false;
				return true;
			}

			// Public data
			uint8_t		m_flags;
		};

	}

}