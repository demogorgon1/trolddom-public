#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct MinionMode
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MINION_MODE;
			static const bool TAGGED = false;

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
					if (!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if (aChild->m_name == "icon")
							m_iconSpriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
						else if (aChild->m_name == "follow_distance")
							m_followDistance = aChild->GetUInt32();
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
				aStream->WriteUInt(m_iconSpriteId);
				aStream->WriteUInt(m_followDistance);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadString(m_string))
					return false;
				if (!aStream->ReadUInt(m_iconSpriteId))
					return false;
				if (!aStream->ReadUInt(m_followDistance))
					return false;
				return true;
			}

			// Public data
			std::string		m_string;
			uint32_t		m_iconSpriteId = 0;
			uint32_t		m_followDistance = 0;
		};

	}

}