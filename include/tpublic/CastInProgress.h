#pragma once

#include "IReader.h"
#include "ItemInstanceReference.h"
#include "IWriter.h"
#include "Vec2.h"

namespace tpublic
{

	class CastInProgress
	{
	public:
		void
		ToStream(
			IWriter*				aWriter) const
		{
			aWriter->WriteUInt(m_abilityId);
			aWriter->WriteUInt(m_targetEntityInstanceId);
			aWriter->WriteInt(m_start);
			aWriter->WriteInt(m_end);
			m_aoeTarget.ToStream(aWriter);
			aWriter->WriteBool(m_channeling);
			m_item.ToStream(aWriter);
		}

		bool
		FromStream(
			IReader*				aReader) 
		{
			if (!aReader->ReadUInt(m_abilityId))
				return false;
			if (!aReader->ReadUInt(m_targetEntityInstanceId))
				return false;
			if (!aReader->ReadInt(m_start))
				return false;
			if (!aReader->ReadInt(m_end))
				return false;
			if(!m_aoeTarget.FromStream(aReader))
				return false;
			if(!aReader->ReadBool(m_channeling))
				return false;
			if(!m_item.FromStream(aReader))
				return false;
			return true;
		}

		bool
		operator==(
			const CastInProgress&	aOther) const
		{
			return m_abilityId == aOther.m_abilityId && m_targetEntityInstanceId == aOther.m_targetEntityInstanceId && m_start == aOther.m_start && 
				m_end == aOther.m_end && m_aoeTarget == aOther.m_aoeTarget && m_channeling == aOther.m_channeling && m_item == aOther.m_item;
		}

		bool
		operator!=(
			const CastInProgress&	aOther) const
		{
			return !this->operator==(aOther);
		}

		// Public data
		uint32_t				m_abilityId = 0;
		uint32_t				m_targetEntityInstanceId = 0;
		int32_t					m_start = 0;
		int32_t					m_end = 0;
		tpublic::Vec2			m_aoeTarget;
		bool					m_channeling = false;
		ItemInstanceReference	m_item;
	};

}