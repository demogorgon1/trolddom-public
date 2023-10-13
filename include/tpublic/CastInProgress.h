#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Vec2.h"

namespace tpublic
{

	class CastInProgress
	{
	public:
		void
		ToStream(
			IWriter*	aWriter) const
		{
			aWriter->WriteUInt(m_abilityId);
			aWriter->WriteUInt(m_targetEntityInstanceId);
			aWriter->WriteInt(m_start);
			aWriter->WriteInt(m_end);
			m_aoeTarget.ToStream(aWriter);
		}

		bool
		FromStream(
			IReader*	aReader) 
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
			return true;
		}

		bool
		operator==(
			const CastInProgress& aOther) const
		{
			return m_abilityId == aOther.m_abilityId && m_targetEntityInstanceId == aOther.m_targetEntityInstanceId && m_start == aOther.m_start && m_end == aOther.m_end && m_aoeTarget == aOther.m_aoeTarget;
		}

		bool
		operator!=(
			const CastInProgress& aOther) const
		{
			return !this->operator==(aOther);
		}

		// Public data
		uint32_t		m_abilityId = 0;
		uint32_t		m_targetEntityInstanceId = 0;
		int32_t			m_start = 0;
		int32_t			m_end = 0;
		tpublic::Vec2	m_aoeTarget;
	};

}