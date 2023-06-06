#pragma once

namespace tpublic
{

	namespace Data
	{
		struct Ability;
	}

	class IReader;
	class IWriter;

	class Cooldowns
	{
	public:
		struct Entry
		{
			void			
			ToStream(
				IWriter*							aStream) const
			{
				aStream->WriteUInt(m_abilityId);
				aStream->WriteIntDelta(aStream->GetTick(), m_start);
				aStream->WriteIntDelta(aStream->GetTick(), m_end);
			}
			
			bool			
			FromStream(
				IReader*							aStream)
			{
				if (!aStream->ReadUInt(m_abilityId))
					return false;
				if (!aStream->ReadIntDelta(aStream->GetTick(), m_start))
					return false;
				if (!aStream->ReadIntDelta(aStream->GetTick(), m_end))
					return false;
				return true;
			}

			// Public data
			uint32_t			m_abilityId = 0;
			int32_t				m_start = 0;
			int32_t				m_end = 0;
		};

						Cooldowns();
						~Cooldowns();		

		bool			Update(
							int32_t					aTick);
		void			Add(
							const Data::Ability*	aAbility,
							int32_t					aTick);
		const Entry*	Get(
							uint32_t				aAbilityId) const;
		void			ToStream(
							IWriter*				aStream) const;
		bool			FromStream(
							IReader*				aStream);

		// Public data
		std::vector<Entry>		m_entries;
	};

}