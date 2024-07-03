#pragma once

#include "../Component.h"
#include "../ComponentBase.h"
#include "../Guid.h"
#include "../Mail.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerOutbox
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_OUTBOX;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_NONE;

			static const size_t OUTBOX_SIZE = 5;

			struct Entry
			{
				void
				ToStream(
					IWriter*	aWriter) const
				{
					aWriter->WritePOD(m_transactionId);
					aWriter->WriteObjectPointer(m_mail);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if(!aReader->ReadPOD(m_transactionId))
						return false;
					if(!aReader->ReadObjectPointer(m_mail))
						return false;
					return true;
				}

				// Public data
				Guid								m_transactionId;
				std::unique_ptr<tpublic::Mail>		m_mail;
			};

			enum Field
			{
				FIELD_OUTGOING
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{	
				aSchema->DefineCustomObjectPointersNoSource<Entry>(FIELD_OUTGOING, offsetof(PlayerOutbox, m_outgoing));
			}	

			void
			Reset()
			{
				m_outgoing.clear();
			}

			// Public data
			std::vector<std::unique_ptr<Entry>>		m_outgoing;
		};
	}

}
