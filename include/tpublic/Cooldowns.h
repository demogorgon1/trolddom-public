#pragma once

namespace tpublic
{

	namespace Data
	{
		struct Ability;
	}

	class AbilityModifierList;
	class IReader;
	class IWriter;
	class Manifest;

	class Cooldowns
	{
	public:
		struct Entry
		{
			void			
			ToStream(
				IWriter*							aStream) const
			{
				aStream->WriteUInt(m_cooldownId);
				
				if(aStream->IsNetworkStream())
				{
					aStream->WriteInt(m_start);
					aStream->WriteInt(m_end);
				}
				else
				{
					// Convert ticks into global UTC time stamps
					int32_t currentTick = aStream->GetTick();
					uint64_t currentTimeStamp = (uint64_t)time(NULL);

					uint64_t startTimeStamp = currentTimeStamp;

					if (m_start < currentTick)
						startTimeStamp -= (uint64_t)(currentTick - m_start) / 10;

					aStream->WriteUInt(startTimeStamp);

					uint64_t endTimeStamp = 0;

					if (m_end > m_start)
						endTimeStamp = startTimeStamp + (uint64_t)(m_end - m_start) / 10;

					aStream->WriteUInt(endTimeStamp);
				}
			}
			
			bool			
			FromStream(
				IReader*							aStream)
			{
				if (!aStream->ReadUInt(m_cooldownId))
					return false;

				if (aStream->IsNetworkStream())
				{
					if(!aStream->ReadInt(m_start))
						return false;
					if (!aStream->ReadInt(m_end))
						return false;
				}
				else
				{
					// Convert global UTC time stamps into ticks
					uint64_t startTimeStamp;
					uint64_t endTimeStamp;

					if (!aStream->ReadUInt(startTimeStamp))
						return false;
					if (!aStream->ReadUInt(endTimeStamp))
						return false;

					int32_t currentTick = aStream->GetTick();
					uint64_t currentTimeStamp = (uint64_t)time(NULL);

					if (startTimeStamp < currentTimeStamp)
						m_start = currentTick - (int32_t)((currentTimeStamp - startTimeStamp) * 10);
					else
						m_start = currentTick;

					if (endTimeStamp != 0)
					{
						assert(endTimeStamp >= startTimeStamp);
						m_end = m_start + (int32_t)((endTimeStamp - startTimeStamp) * 10);
					}
					else
					{
						m_end = 0;
					}
				}

				return true;
			}

			// Public data
			uint32_t			m_cooldownId = 0;
			int32_t				m_start = 0;
			int32_t				m_end = 0;
		};

						Cooldowns();
						~Cooldowns();		

		bool			Update(
							int32_t						aTick);
		void			AddAbility(
							const Manifest*				aManifest,
							const Data::Ability*		aAbility,
							int32_t						aTick,
							float						aCooldownModifier,
							const AbilityModifierList*	aAbilityModifierList);
		void			AddCooldown(
							uint32_t					aCooldownId,
							int32_t						aDuration,
							int32_t						aTick);
		bool			IsAbilityOnCooldown(
							const Data::Ability*		aAbility) const;
		const Entry*	GetCooldown(
							uint32_t					aCooldownId) const;
		void			ToStream(
							IWriter*					aStream) const;
		bool			FromStream(
							IReader*					aStream);

		// Public data
		std::vector<Entry>		m_entries;
	};

}