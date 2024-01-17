#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerInbox
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_INBOX;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_PRIVATE;

			struct Mail
			{
				void
				ToStream(
					IWriter*	aWriter) const
				{
					aWriter->WriteUInt(m_serverTimeStamp);
					aWriter->WriteString(m_sender);
					aWriter->WriteString(m_subject);
					aWriter->WriteString(m_body);
					aWriter->WriteObjects(m_items);
					aWriter->WriteInt(m_cash);
					aWriter->WriteBool(m_read);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if (!aReader->ReadUInt(m_serverTimeStamp))
						return false;
					if (!aReader->ReadString(m_sender))
						return false;
					if (!aReader->ReadString(m_subject))
						return false;
					if (!aReader->ReadString(m_body))
						return false;
					if (!aReader->ReadObjects(m_items))
						return false;
					if (!aReader->ReadInt(m_cash))
						return false;
					if (!aReader->ReadBool(m_read))
						return false;
					return true;
				}

				// Public data
				uint64_t							m_serverTimeStamp = 0;
				std::string							m_sender;
				std::string							m_subject;
				std::string							m_body;
				std::vector<tpublic::ItemInstance>	m_items;
				int64_t								m_cash = 0;
				bool								m_read = false;
			};

			enum Field
			{
				FIELD_MAILS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{	
				aSchema->DefineCustomObjectPointersNoSource<Mail>(FIELD_MAILS, offsetof(PlayerInbox, m_mails));
			}

			// Public data
			std::vector<std::unique_ptr<Mail>>		m_mails;
		};
	}

}