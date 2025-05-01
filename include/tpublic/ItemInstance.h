#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	struct ItemInstance
	{
		enum Flag : uint8_t
		{
			FLAG_SOULBOUND = 0x01,
			FLAG_ENCHANTED = 0x02
		};

		ItemInstance(
			uint32_t		aItemId = 0,
			uint32_t		aQuantity = 1,
			uint32_t		aExtraId = 0,
			uint8_t			aFlags = 0)
			: m_itemId(aItemId)
			, m_quantity(aQuantity)
			, m_flags(aFlags)
			, m_extraId(aExtraId)
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
		IsEnchanted() const
		{
			return m_flags & FLAG_ENCHANTED;
		}

		void
		SetEnchanted(
			uint32_t		aEnchantAuraId)
		{
			m_flags |= FLAG_ENCHANTED;
			m_extraId = aEnchantAuraId;
		}

		bool
		IsWorldbound() const
		{
			return !IsEnchanted() && m_extraId != 0;
		}

		void
		SetWorldbound(
			uint32_t		aCharacterId)
		{
			if(!IsEnchanted())
				m_extraId = aCharacterId;
		}

		uint32_t
		GetWorldboundCharacterId() const
		{
			return m_extraId;
		}

		void
		Clear()
		{
			m_itemId = 0;
			m_quantity = 1;
			m_extraId = 0;
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
				aWriter->WriteUInt(m_extraId);
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
				if (!aReader->ReadUInt(m_extraId))
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
			return m_itemId == aOther.m_itemId && m_quantity == aOther.m_quantity && m_flags == aOther.m_flags && m_extraId == aOther.m_extraId;
		}

		bool
		operator !=(
			const ItemInstance&	aOther) const
		{
			return !this->operator==(aOther);
		}

		// Public data
		uint32_t		m_itemId = 0;
		uint32_t		m_quantity = 0;
		uint32_t		m_extraId = 0;
		uint8_t			m_flags = 0;
	};

}