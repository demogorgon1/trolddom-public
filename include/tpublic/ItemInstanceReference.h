#pragma once

#include "IReader.h"
#include "ItemInstance.h"
#include "IWriter.h"

namespace tpublic
{

	struct ItemInstanceReference
	{
		enum Type : uint8_t
		{
			TYPE_NONE,
			TYPE_INVENTORY,
			TYPE_EQUIPMENT
		};

		void
		ToStream(
			IWriter*						aWriter) const
		{
			aWriter->WritePOD(m_type);
			if (m_type != TYPE_NONE)
			{
				aWriter->WriteUInt(m_index);
				m_itemInstance.ToStream(aWriter);
			}
		}

		bool
		FromStream(
			IReader*						aReader)
		{
			if (!aReader->ReadPOD(m_type))
				return false;
			if (m_type != TYPE_NONE)
			{
				if (!aReader->ReadUInt(m_index))
					return false;
				if (!m_itemInstance.FromStream(aReader))
					return false;
			}
			return true;
		}

		bool
		operator==(
			const ItemInstanceReference&	aOther) const
		{
			return m_type == aOther.m_type && m_index == aOther.m_index && m_itemInstance == aOther.m_itemInstance;
		}

		// Public data
		Type			m_type = TYPE_NONE;
		uint32_t		m_index = 0;
		ItemInstance	m_itemInstance;
	};

}
