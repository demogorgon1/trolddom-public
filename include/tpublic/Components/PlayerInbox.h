#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../Mail.h"

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

			static const size_t INBOX_SIZE = 20;

			struct Entry
			{
				void
				ToStream(
					IWriter*	aWriter) const
				{
					aWriter->WriteUInt(m_mailId);
					aWriter->WriteUInt(m_sendTimeStamp);
					aWriter->WriteUInt(m_recvTimeStamp);
					aWriter->WriteBool(m_read);
					aWriter->WriteObjectPointer(m_mail);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if (!aReader->ReadUInt(m_mailId))
						return false;
					if (!aReader->ReadUInt(m_sendTimeStamp))
						return false;
					if (!aReader->ReadUInt(m_recvTimeStamp))
						return false;
					if (!aReader->ReadBool(m_read))
						return false;
					if(!aReader->ReadObjectPointer(m_mail))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_mailId = 0;
				uint64_t							m_sendTimeStamp = 0;
				uint64_t							m_recvTimeStamp = 0;
				bool								m_read = false;
				std::unique_ptr<tpublic::Mail>		m_mail;
			};

			enum Field
			{
				FIELD_MAILS,
				FIELD_INCOMING,
				FIELD_LAST_CHECK_TIME,
				FIELD_SEQ
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{	
				aSchema->DefineCustomObjectPointersNoSource<Entry>(FIELD_MAILS, offsetof(PlayerInbox, m_mails));
				aSchema->DefineCustomObjectPointersNoSource<Entry>(FIELD_INCOMING, offsetof(PlayerInbox, m_incoming));
				aSchema->Define(ComponentSchema::TYPE_UINT64, FIELD_LAST_CHECK_TIME, NULL, offsetof(PlayerInbox, m_lastCheckTime));
				aSchema->Define(ComponentSchema::TYPE_UINT32, FIELD_SEQ, NULL, offsetof(PlayerInbox, m_seq));
			}

			size_t
			GetIncomingIndexByMailId(
				uint32_t		aMailId) const
			{
				for(size_t i = 0; i < m_incoming.size(); i++)
				{
					if(m_incoming[i]->m_mailId == aMailId)
						return i;
				}
				return SIZE_MAX;
			}

			bool
			IsFull() const
			{
				return m_mails.size() + m_incoming.size() >= INBOX_SIZE;
			}

			const Entry*
			GetMail(
				size_t			aIndex) const
			{
				if(aIndex >= m_mails.size())
					return NULL;
				return m_mails[aIndex].get();
			}

			Entry*
			GetMail(
				size_t			aIndex) 
			{
				if(aIndex >= m_mails.size())
					return NULL;
				return m_mails[aIndex].get();
			}

			size_t
			GetMailIndexByMailId(
				uint32_t		aMailId) const
			{
				for(size_t i = 0; i < m_mails.size(); i++)
				{
					if(m_mails[i]->m_mailId == aMailId)
						return i;
				}
				return SIZE_MAX;
			}

			// Public data
			std::vector<std::unique_ptr<Entry>>		m_mails;
			std::vector<std::unique_ptr<Entry>>		m_incoming;
			uint64_t								m_lastCheckTime = 0;
			uint32_t								m_seq = 0;
		};
	}

}
