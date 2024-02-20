#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class PlayerGateways
	{
	public:
		struct LockedSeed
		{
			void
			ToStream(
				IWriter*	aWriter) const
			{
				aWriter->WriteUInt(m_seed);
				aWriter->WriteUInt(m_expirationTimeStamp);
			}

			bool
			FromStream(
				IReader*	aReader)
			{
				if (!aReader->ReadUInt(m_seed))
					return false;
				if (!aReader->ReadUInt(m_expirationTimeStamp))
					return false;
				return true;
			}

			// Public data
			uint32_t				m_seed = 0;
			uint64_t				m_expirationTimeStamp = 0;
		};

		void
		AddLockedSeed(
			uint32_t		aSeed,
			uint64_t		aExpirationTimeStamp)
		{
			if(!IsLockedSeed(aSeed))
				m_lockedSeeds.push_back({ aSeed, aExpirationTimeStamp });
		}

		bool
		IsLockedSeed(
			uint32_t		aSeed) const
		{
			for (const LockedSeed& t : m_lockedSeeds)
			{
				if (t.m_seed == aSeed)
					return true;
			}
			return false;
		}

		void
		ToStream(
			IWriter*		aWriter) const
		{
			aWriter->WriteUInt(m_currentSeed);
			aWriter->WriteObjects(m_lockedSeeds);
		}

		bool
		FromStream(
			IReader*		aReader)
		{
			if (!aReader->ReadUInt(m_currentSeed))
				return false;
			if (!aReader->ReadObjects(m_lockedSeeds))
				return false;
			return true;
		}
	
		// Public data
		uint32_t					m_currentSeed = 0;
		std::vector<LockedSeed>		m_lockedSeeds;
	};

}