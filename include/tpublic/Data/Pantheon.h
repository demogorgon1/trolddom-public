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
				NOTIFICATION_STRING_GAINED_FAVOR,
				NOTIFICATION_STRING_LOST_FAVOR,

				NUM_NOTIFICATION_STRINGS
			};

			static NotificationString
			SourceToNotificationString(
				const SourceNode*		aSource)
			{
				TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing notification string annotation.");
				std::string_view t(aSource->m_annotation->GetIdentifier());
				if(t == "gained_favor")
					return NOTIFICATION_STRING_GAINED_FAVOR;
				else if(t == "lost_favor")
					return NOTIFICATION_STRING_LOST_FAVOR;				
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid notification string.", aSource->m_annotation->GetIdentifier());
				return NotificationString(0);
			}

			void
			Verify() const
			{
				VerifyBase();
			}

			const char*
			GetPlayerLevelString(
				uint32_t				aLevel) const
			{
				if(m_playerLevels.empty())
					return "";

				uint32_t index = aLevel == UINT32_MAX ? 0 : aLevel + 1;
				if(index >= (uint32_t)m_playerLevels.size())
					index = (uint32_t)m_playerLevels.size() - 1;

				return m_playerLevels[index].c_str();
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
						else if (aChild->m_name == "deity_specifier")
							m_deitySpecifier = aChild->GetString();
						else if (aChild->m_name == "shrine_display_name_prefix")
							m_shrineDisplayNamePrefix = aChild->GetString();
						else if(aChild->m_name == "faction")
							m_factionId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_FACTION, aChild->GetIdentifier());
						else if (aChild->m_name == "icon")
							m_iconSpriteId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SPRITE, aChild->GetIdentifier());
						else if (aChild->m_name == "pray_ability")
							m_prayAbilityId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aChild->GetIdentifier());
						else if (aChild->m_name == "opposition")
							m_oppositionPantheonId = aChild->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_PANTHEON, aChild->GetIdentifier());
						else if(aChild->m_name == "notification_string")
							m_notificationStrings[SourceToNotificationString(aChild)] = aChild->GetString();
						else if(aChild->m_name == "player_levels")
							aChild->GetStringArray(m_playerLevels);
						else if (aChild->m_name == "blessings")
							aChild->GetIdArray(DataType::ID_AURA, m_blessingAuraIds);
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
				aWriter->WriteUInt(m_iconSpriteId);
				aWriter->WriteString(m_deitySpecifier);
				aWriter->WriteUInt(m_factionId);
				aWriter->WriteUInt(m_oppositionPantheonId);
				aWriter->WriteUInt(m_prayAbilityId);
				aWriter->WriteStrings(m_playerLevels);
				aWriter->WriteUInts(m_blessingAuraIds);
				aWriter->WriteString(m_shrineDisplayNamePrefix);

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
				if (!aReader->ReadUInt(m_iconSpriteId))
					return false;
				if (!aReader->ReadString(m_deitySpecifier))
					return false;
				if (!aReader->ReadUInt(m_factionId))
					return false;
				if (!aReader->ReadUInt(m_oppositionPantheonId))
					return false;
				if (!aReader->ReadUInt(m_prayAbilityId))
					return false;
				if (!aReader->ReadStrings(m_playerLevels))
					return false;
				if (!aReader->ReadUInts(m_blessingAuraIds))
					return false;
				if(!aReader->ReadString(m_shrineDisplayNamePrefix))
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
			uint32_t							m_iconSpriteId = 0;
			uint32_t							m_factionId = 0;
			std::string							m_deitySpecifier;
			uint32_t							m_oppositionPantheonId = 0;
			std::string							m_notificationStrings[NUM_NOTIFICATION_STRINGS];
			std::vector<std::string>			m_playerLevels;
			std::vector<uint32_t>				m_blessingAuraIds;
			uint32_t							m_prayAbilityId = 0;
			std::string							m_shrineDisplayNamePrefix;
		};

	}

}