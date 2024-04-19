#pragma once

#include "../Data/Aura.h"

#include "../AuraEffectBase.h"
#include "../ComponentBase.h"
#include "../Manifest.h"
#include "../MoveSpeed.h"

namespace tpublic
{

	namespace Components
	{

		struct Auras
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_AURAS;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_VOLATILE;
			static const Replication REPLICATION = REPLICATION_NONE;

			struct Entry
			{
				void
				ToStream(
					IWriter*					aWriter) const
				{
					aWriter->WriteUInt(m_auraId);

					// Convert ticks into global UTC time stamps
					int32_t currentTick = aWriter->GetTick();
					uint64_t currentTimeStamp = (uint64_t)time(NULL);

					uint64_t startTimeStamp = currentTimeStamp;

					if(m_start < currentTick)
						startTimeStamp -= (uint64_t)(currentTick - m_start) / 10;

					aWriter->WriteUInt(startTimeStamp);

					uint64_t endTimeStamp = 0;

					if(m_end != 0 && m_end > m_start)
						endTimeStamp = startTimeStamp + (uint64_t)(m_end - m_start) / 10;

					aWriter->WriteUInt(endTimeStamp);
				}

				bool
				FromStream(
					IReader*					aReader) 
				{
					if (!aReader->ReadUInt(m_auraId))
						return false;
					
					// Convert global UTC time stamps into ticks
					uint64_t startTimeStamp;
					uint64_t endTimeStamp;

					if (!aReader->ReadUInt(startTimeStamp))
						return false;
					if (!aReader->ReadUInt(endTimeStamp))
						return false;

					int32_t currentTick = aReader->GetTick();
					uint64_t currentTimeStamp = (uint64_t)time(NULL);

					if(startTimeStamp < currentTimeStamp)
						m_start = currentTick - (int32_t)((currentTimeStamp - startTimeStamp) * 10);
					else
						m_start = currentTick;

					if(endTimeStamp != 0)
					{
						assert(endTimeStamp >= startTimeStamp);
						m_end = m_start + (int32_t)((endTimeStamp - startTimeStamp) * 10);
					}
					else
					{
						m_end = 0;
					}

					return true;
				}

				bool
				HasEffect(
					AuraEffect::Id				aId) const
				{
					for (const std::unique_ptr<AuraEffectBase>& effect : m_effects)
					{
						if (effect->m_id == aId)
							return true;
					}
					return false;
				}

				// Public data
				uint32_t										m_auraId = 0;
				int32_t											m_start = 0;
				int32_t											m_end = 0;

				// Not serialized
				uint32_t										m_entityInstanceId = 0; 
				uint32_t										m_channeledAbilityId = 0;	
				bool											m_cancel = false;
				bool											m_noEffects = false;
				std::vector<std::unique_ptr<AuraEffectBase>>	m_effects; 
			};

			enum Field
			{
				FIELD_ENTRIES
			};

			static void
			CreateSchema(
				ComponentSchema* aSchema)
			{
				aSchema->DefineCustomObjectPointersNoSource<Entry>(FIELD_ENTRIES, offsetof(Auras, m_entries));

				aSchema->OnRead<Auras>([](
					Auras*						aAuras,
					ComponentSchema::ReadType	aReadType,
					const Manifest*				aManifest)
				{
					if(aReadType == ComponentSchema::READ_TYPE_STORAGE)
						aAuras->OnLoadedFromPersistence(aManifest);
				});
			}

			bool			HasEffect(
								AuraEffect::Id								aId) const;
			MoveSpeed::Id	GetMoveSpeed() const;
			int32_t			FilterDamageInput(
								DirectEffect::DamageType					aDamageType,
								int32_t										aDamage) const;
			int32_t			FilterDamageOutput(
								DirectEffect::DamageType					aDamageType,
								int32_t										aDamage) const;
			int32_t			FilterHealInput(
								int32_t										aHeal) const;
			void			OnCombatEvent(
								tpublic::AuraEffectBase::CombatEventType	aType,
								CombatEvent::Id								aCombatEventId,
								AuraEffectBase::SecondaryAbilityCallback	aCallback) const;
			void			RemoveAura(
								uint32_t									aAuraId);
			void			OnLoadedFromPersistence(
								const Manifest*								aManifest);

			// Public data
			std::vector<std::unique_ptr<Entry>>					m_entries;
			uint32_t											m_seq = 0;
		};
	}

}