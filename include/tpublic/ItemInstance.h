#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	struct ItemInstance
	{
		ItemInstance(
			uint32_t		aItemId = 0,
			uint32_t		aQuantity = 1,
			bool			aSoulbound = false)
			: m_itemId(aItemId)
			, m_quantity(aQuantity)
			, m_soulbound(aSoulbound)
		{

		}

		bool	
		IsSet() const
		{
			return m_itemId != 0;
		}

		void
		Clear()
		{
			m_itemId = 0;
			m_quantity = 1;
			m_soulbound = false;
		}

		void
		ToStream(
			IWriter*		aWriter) const
		{
			aWriter->WriteUInt(m_itemId);
			if(IsSet())
			{
				aWriter->WriteUInt(m_quantity);
				aWriter->WriteBool(m_soulbound);
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
				if(!aReader->ReadBool(m_soulbound))
					return false;
			}
			return true;
		}

		bool
		operator ==(
			const ItemInstance&	aOther) const
		{
			return m_itemId == aOther.m_itemId && m_quantity == aOther.m_quantity && m_soulbound == aOther.m_soulbound;
		}

		bool
		operator !=(
			const ItemInstance&	aOther) const
		{
			return !this->operator==(aOther);
		}

		ItemInstance&
		operator=(
			const ItemInstance& aOther) 
		{
			m_itemId = aOther.m_itemId;
			m_quantity = aOther.m_quantity;
			m_soulbound = aOther.m_soulbound;
			return *this;
		}

		// Public data
		uint32_t		m_itemId;
		uint32_t		m_quantity;
		bool			m_soulbound;
	};

}