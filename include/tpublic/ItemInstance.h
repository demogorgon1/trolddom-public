#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	struct ItemInstance
	{
		bool	
		IsSet() const
		{
			return m_itemId != 0;
		}

		void
		Clear()
		{
			m_itemId = 0;
			m_seed = 0;
			m_quantity = 1;
		}

		void
		ToStream(
			IWriter*		aWriter) const
		{
			aWriter->WriteUInt(m_itemId);
			if(IsSet())
			{
				aWriter->WritePOD(m_seed);
				aWriter->WriteUInt(m_quantity);
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
				if(!aReader->ReadPOD(m_seed))
					return false;
				if (!aReader->ReadUInt(m_quantity))
					return false;
			}
			return true;
		}

		bool
		operator ==(
			const ItemInstance&	aOther) const
		{
			return m_itemId == aOther.m_itemId && m_seed == aOther.m_seed && m_quantity == aOther.m_quantity;
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
			m_seed = aOther.m_seed;
			m_quantity = aOther.m_quantity;
			return *this;
		}

		// Public data
		uint32_t		m_itemId = 0;
		uint32_t		m_seed = 0;
		uint32_t		m_quantity = 1;
	};

}