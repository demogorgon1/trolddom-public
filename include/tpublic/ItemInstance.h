#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	struct ItemInstance
	{
		enum Flag : uint8_t
		{
			FLAG_SOULBOUND = 0x01
		};

		ItemInstance(
			uint32_t		aItemId = 0,
			uint32_t		aQuantity = 1,
			uint32_t		aWorldboundCharacterId = 0,
			uint8_t			aFlags = 0)
			: m_itemId(aItemId)
			, m_quantity(aQuantity)
			, m_worldboundCharacterId(aWorldboundCharacterId)
			, m_flags(aFlags)
		{

		}

		bool	
		IsSet() const
		{
			return m_itemId != 0;
		}

		bool
		IsSoulbound() const
		{
			return m_flags & FLAG_SOULBOUND;
		}

		void
		SetSoulbound()
		{
			m_flags |= FLAG_SOULBOUND;
		}

		bool
		IsWorldbound() const
		{
			return m_worldboundCharacterId != 0;
		}

		void
		SetWorldbound(
			uint32_t		aCharacterId)
		{
			m_worldboundCharacterId = aCharacterId;
		}

		void
		Clear()
		{
			m_itemId = 0;
			m_quantity = 1;
			m_worldboundCharacterId = 0;
			m_flags = 0;
		}

		void
		ToStream(
			IWriter*		aWriter) const
		{
			aWriter->WriteUInt(m_itemId);
			if(IsSet())
			{
				aWriter->WriteUInt(m_quantity);
				aWriter->WriteUInt(m_worldboundCharacterId);
				aWriter->WritePOD(m_flags);
			}
		}

		bool
		FromStream(
			IReader*		aReader)
		{
			if(!aReader->ReadUInt(m_itemId))
				return false;
			if(IsSet())
			{
				if (!aReader->ReadUInt(m_quantity))
					return false;
				if (!aReader->ReadUInt(m_worldboundCharacterId))
					return false;
				if(!aReader->ReadPOD(m_flags))
					return false;
			}
			return true;
		}

		bool
		operator ==(
			const ItemInstance&	aOther) const
		{
			return m_itemId == aOther.m_itemId && m_quantity == aOther.m_quantity && m_flags == aOther.m_flags && m_worldboundCharacterId == aOther.m_worldboundCharacterId;
		}

		bool
		operator !=(
			const ItemInstance&	aOther) const
		{
			return !this->operator==(aOther);
		}

		// Public data
		uint32_t		m_itemId;
		uint32_t		m_quantity;
		uint32_t		m_worldboundCharacterId;
		uint8_t			m_flags;
	};

}