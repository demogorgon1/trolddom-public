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
							m_iconSpriteId = aChild->GetId(DataType::ID_SPRITE);
						else if (aChild->m_name == "follow_distance")
							m_followDistance = aChild->GetUInt32();
						else if (aChild->m_name == "aggro_range")
							m_aggroRange = aChild->GetUInt32();
						else if (aChild->m_name == "attack_owner_threat_target")
							m_attackOwnerThreatTarget = aChild->GetBool();
						else if (aChild->m_name == "attack_threat_target")
							m_attackThreatTarget = aChild->GetBool();
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
				aStream->WriteUInt(m_aggroRange);
				aStream->WriteBool(m_attackOwnerThreatTarget);
				aStream->WriteBool(m_attackThreatTarget);
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
				if (!aStream->ReadUInt(m_aggroRange))
					return false;
				if (!aStream->ReadBool(m_attackOwnerThreatTarget))
					return false;
				if (!aStream->ReadBool(m_attackThreatTarget))
					return false;
				return true;
			}

			// Public data
			std::string		m_string;
			uint32_t		m_iconSpriteId = 0;
			uint32_t		m_followDistance = 0;
			uint32_t		m_aggroRange = 0;
			bool			m_attackOwnerThreatTarget = false;
			bool			m_attackThreatTarget = false;
		};

	}

}