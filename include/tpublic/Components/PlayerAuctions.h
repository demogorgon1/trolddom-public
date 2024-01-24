#pragma once

#include "../AuctionDuration.h"
#include "../Component.h"
#include "../ComponentBase.h"
#include "../Guid.h"

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
					aWriter->WritePOD(m_transactionId);
					aWriter->WriteUInt(m_itemId);
					aWriter->WriteUInt(m_stackSize);
					aWriter->WriteInt(m_startPrice);
					aWriter->WriteInt(m_buyoutPrice);
					aWriter->WritePOD(m_duration);
					aWriter->WriteInt(m_deposit);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if(!aReader->ReadPOD(m_transactionId))
						return false;
					if (!aReader->ReadUInt(m_itemId))
						return false;
					if (!aReader->ReadUInt(m_stackSize))
						return false;
					if (!aReader->ReadInt(m_startPrice))
						return false;
					if (!aReader->ReadInt(m_buyoutPrice))
						return false;
					if (!aReader->ReadPOD(m_duration))
						return false;
					if (!aReader->ReadInt(m_deposit))
						return false;
					return true;
				}

				// Public data
				Guid								m_transactionId;
				uint32_t							m_itemId = 0;
				uint32_t							m_stackSize = 0;
				int64_t								m_startPrice = 0;
				int64_t								m_buyoutPrice = 0;
				AuctionDuration::Id					m_duration;
				int64_t								m_deposit = 0;
			};

			struct CancelAuction
			{
				void
				ToStream(
					IWriter*	aWriter) const
				{
					aWriter->WriteUInt(m_auctionId);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if (!aReader->ReadUInt(m_auctionId))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_auctionId = 0;
			};

			struct Bid
			{
				void
				ToStream(
					IWriter*	aWriter) const
				{
					aWriter->WriteUInt(m_auctionId);
					aWriter->WriteInt(m_cash);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if (!aReader->ReadUInt(m_auctionId))
						return false;
					if (!aReader->ReadInt(m_cash))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_auctionId = 0;
				int64_t								m_cash = 0;
			};

			struct Buyout
			{
				void
				ToStream(
					IWriter*	aWriter) const
				{
					aWriter->WriteUInt(m_auctionId);
					aWriter->WriteInt(m_cash);
				}

				bool
				FromStream(
					IReader*	aReader)
				{
					if (!aReader->ReadUInt(m_auctionId))
						return false;
					if (!aReader->ReadInt(m_cash))
						return false;
					return true;
				}

				// Public data
				uint32_t							m_auctionId = 0;
				int64_t								m_cash = 0;
			};

			enum Field
			{
				FIELD_PENDING_CREATE_AUCTIONS,
				FIELD_PENDING_CANCEL_AUCTIONS,
				FIELD_PENDING_BIDS,
				FIELD_PENDING_BUYOUTS
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{	
				aSchema->DefineCustomObjectsNoSource<CreateAuction>(FIELD_PENDING_CREATE_AUCTIONS, offsetof(PlayerAuctions, m_pendingCreateAuctions));
				aSchema->DefineCustomObjectsNoSource<CancelAuction>(FIELD_PENDING_CANCEL_AUCTIONS, offsetof(PlayerAuctions, m_pendingCancelAuctions));
				aSchema->DefineCustomObjectsNoSource<Bid>(FIELD_PENDING_BIDS, offsetof(PlayerAuctions, m_pendingBids));
				aSchema->DefineCustomObjectsNoSource<Buyout>(FIELD_PENDING_BUYOUTS, offsetof(PlayerAuctions, m_pendingBuyouts));
			}

			bool
			HasPendingCancelAuction(
				uint32_t		aAuctionId) const
			{
				for(const CancelAuction& t : m_pendingCancelAuctions)
				{
					if(t.m_auctionId == aAuctionId)
						return true;
				}
				return false;
			}

			bool
			HasPendingBid(
				uint32_t		aAuctionId) const
			{
				for(const Bid& t : m_pendingBids)
				{
					if(t.m_auctionId == aAuctionId)
						return true;
				}
				return false;
			}

			bool
			HasPendingBuyout(
				uint32_t		aAuctionId) const
			{
				for(const Buyout& t : m_pendingBuyouts)
				{
					if(t.m_auctionId == aAuctionId)
						return true;
				}
				return false;
			}

			// Public data
			std::vector<CreateAuction>		m_pendingCreateAuctions;
			std::vector<CancelAuction>		m_pendingCancelAuctions;
			std::vector<Bid>				m_pendingBids;
			std::vector<Buyout>				m_pendingBuyouts;
		};
	}

}