#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct TalentTree
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_TALENT_TREE;

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if(aChild->m_name == "string")
						m_string = aChild->GetString();
					else if (aChild->m_name == "icon")
						m_iconSpriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
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
				aStream->WriteUInt(m_iconSpriteId);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadString(m_string))
					return false;
				if(!aStream->ReadUInt(m_iconSpriteId))
					return false;
				return true;
			}

			// Public data
			std::string		m_string;
			uint32_t		m_iconSpriteId = 0;
		};

	}

}