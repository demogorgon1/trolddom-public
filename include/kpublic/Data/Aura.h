#pragma once

#include "../DataBase.h"

namespace kpublic
{

	namespace Data
	{

		struct Aura
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_AURA;

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
					if(aChild->m_name == "icon")
						m_iconSpriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
					else
						KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteUInt(m_iconSpriteId);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadUInt(m_iconSpriteId))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_iconSpriteId = 0;
		};

	}

}