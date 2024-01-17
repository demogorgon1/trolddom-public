#pragma once

#include "../Component.h"
#include "../ComponentBase.h"

namespace tpublic
{

	namespace Components
	{

		struct PlayerAuctions
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_PLAYER_AUCTIONS;
			static const uint8_t FLAGS = FLAG_PLAYER_ONLY;
			static const Persistence::Id PERSISTENCE = Persistence::ID_MAIN;
			static const Replication REPLICATION = REPLICATION_NONE;

			struct CreateAuction
			{
				void
				ToStream(
					IWriter*	aWriter) const
				{
					aWriter->WriteUInt(m_itemId);
					aWriter->WriteUInt(m_stackSize);
					aWriter->WriteUInt(m_stackCount);
					aWriter->WriteInt(m_startPrice);
					aWriter->WriteInt(m_buyoutPrice);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if (!aReader->ReadUInt(m_itemId))
						return false;
					if (!aReader->ReadUInt(m_stackSize))
						return false;
					if (!aReader->ReadUInt(m_stackCount))
						return false;
					if (!aReader->ReadInt(m_startPrice))
						return false;
					if (!aReader->ReadInt(m_buyoutPrice))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_itemId = 0;
				uint32_t							m_stackSize = 0;
				uint32_t							m_stackCount = 0;
				int64_t								m_startPrice = 0;
				int64_t								m_buyoutPrice = 0;
			};

			enum Field
			{
				FIELD_PENDING_CREATE_AUCTIONS,
				FIELD_CREATE_AUCTIONS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{	
				aSchema->DefineCustomObjectsNoSource<CreateAuction>(FIELD_PENDING_CREATE_AUCTIONS, offsetof(PlayerAuctions, m_pendingCreateAuctions));
				aSchema->DefineCustomObjectsNoSource<CreateAuction>(FIELD_CREATE_AUCTIONS, offsetof(PlayerAuctions, m_createAuctions));
			}

			// Public data
			std::vector<CreateAuction>		m_pendingCreateAuctions;
			std::vector<CreateAuction>		m_createAuctions;

			// Internal
			std::atomic_bool				m_creatingAuctions;
		};
	}

}