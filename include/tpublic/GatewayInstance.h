#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "TimeSeed.h"

namespace tpublic
{

	class GatewayInstance
	{
	public:
		void
		ToStream(
			IWriter*		aWriter) const 
		{
			m_timeSeed.ToStream(aWriter);
			aWriter->WriteUInt(m_mapId);
		}

		bool
		FromStream(
			IReader*		aReader)
		{
			if(!m_timeSeed.FromStream(aReader))
				return false;
			if (!aReader->ReadUInt(m_mapId))
				return false;
			return true;
		}

		bool
		IsSet() const
		{
			return m_mapId != 0;
		}

		// Public data
		TimeSeed		m_timeSeed;
		uint32_t		m_mapId = 0;	
	};
	
}