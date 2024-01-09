#pragma once

#include "../DataBase.h"
#include "../Stat.h"

namespace tpublic
{

	namespace Data
	{

		struct Pantheon
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_PANTHEON;
			static const bool TAGGED = true;

			enum NotificationString : uint8_t
			{
				NOTIFICATION_STRING_GAINED_REPUTATION,
				NOTIFICATION_STRING_LOST_REPUTATION,

				NUM_NOTIFICATION_STRINGS
			};

			static NotificationString
			SourceToNotificationString(
				const SourceNode*		aSource)
			{
				TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing notification string annotation.");
				std::string_view t(aSource->m_annotation->GetIdentifier());
				if(t == "gained_reputation")
					return NOTIFICATION_STRING_GAINED_REPUTATION;
				else if(t == "lost_reputation")
					return NOTIFICATION_STRING_LOST_REPUTATION;				
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid notification string.", aSource->m_annotation->GetIdentifier());
				return NotificationString(0);
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
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
							m_string = aChild->GetString();
						else if(aChild->m_name == "faction")
							m_factionId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_FACTION, aChild->GetIdentifier());
						else if (aChild->m_name == "opposition")
							m_oppositionPantheonId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PANTHEON, aChild->GetIdentifier());
						else if(aChild->m_name == "notification_string")
							m_notificationStrings[SourceToNotificationString(aChild)] = aChild->GetString();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void	
			ToStream(
				IWriter*				aWriter) const override
			{
				ToStreamBase(aWriter);

				aWriter->WriteString(m_string);
				aWriter->WriteUInt(m_factionId);
				aWriter->WriteUInt(m_oppositionPantheonId);

				for (uint32_t i = 0; i < (uint32_t)NUM_NOTIFICATION_STRINGS; i++)
					aWriter->WriteString(m_notificationStrings[i]);
			}
			
			bool
			FromStream(
				IReader*				aReader) override
			{
				if (!FromStreamBase(aReader))
					return false;

				if(!aReader->ReadString(m_string))
					return false;
				if (!aReader->ReadUInt(m_factionId))
					return false;
				if (!aReader->ReadUInt(m_oppositionPantheonId))
					return false;

				for(uint32_t i = 0; i < (uint32_t)NUM_NOTIFICATION_STRINGS; i++)
				{
					if(!aReader->ReadString(m_notificationStrings[i]))
						return false;
				}
				return true;
			}

			// Public data
			std::string							m_string;
			uint32_t							m_factionId = 0;
			uint32_t							m_oppositionPantheonId = 0;
			std::string							m_notificationStrings[NUM_NOTIFICATION_STRINGS];
		};

	}

}