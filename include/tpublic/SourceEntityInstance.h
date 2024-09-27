#pragma once

namespace tpublic
{

	class SourceEntityInstance
	{
	public:
		struct Hasher
		{
			uint32_t
			operator()(
				const SourceEntityInstance&	aKey) const
			{
				return (uint32_t)Hash::Splitmix_2_32(aKey.m_entityInstanceSeq, (uint32_t)aKey.m_entityInstanceSeq);
			}
		};

		bool
		operator==(
			const SourceEntityInstance&		aOther) const
		{
			return m_entityInstanceSeq == aOther.m_entityInstanceSeq && m_entityInstanceId == aOther.m_entityInstanceId;
		}

		bool
		IsSet() const
		{
			return m_entityInstanceId != 0;
		}

		// Public data
		uint32_t					m_entityInstanceId = 0;
		uint8_t						m_entityInstanceSeq = 0;
	};

}