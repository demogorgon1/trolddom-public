#pragma once

#include "../AuraEffectBase.h"

namespace tpublic
{

	namespace AuraEffects
	{

		struct DamageInputEffect
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_DAMAGE_INPUT_EFFECT;
			
			enum TargetFlag : uint8_t
			{
				TARGET_FLAG_SOURCE					= 0x01,
				TARGET_FLAG_SOURCE_COMBAT_GROUP		= 0x02,
				TARGET_FLAG_SOURCE_MINIONS			= 0x04
			};

			enum EffectType : uint8_t
			{
				INVALID_EFFECT_TYPE,

				EFFECT_TYPE_RESTORE_HEALTH,
				EFFECT_TYPE_RESTORE_MANA
			};

			static uint8_t
			SourceToTargetFlags(
				const SourceNode*			aSource)
			{
				uint8_t targetFlags = 0;
				aSource->GetArray()->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->IsIdentifier("source"))
						targetFlags |= TARGET_FLAG_SOURCE;
					else if (aChild->IsIdentifier("source_combat_group"))
						targetFlags |= TARGET_FLAG_SOURCE_COMBAT_GROUP;
					else if (aChild->IsIdentifier("source_minions"))
						targetFlags |= TARGET_FLAG_SOURCE_MINIONS;
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid target flag.", aChild->GetIdentifier());
				});
				return targetFlags;
			}

			static EffectType
			SourceToEffectType(
				const SourceNode*			aSource)
			{
				if (aSource->IsIdentifier("restore_health"))
					return EFFECT_TYPE_RESTORE_HEALTH;
				else if (aSource->IsIdentifier("restore_mana"))
					return EFFECT_TYPE_RESTORE_MANA;
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid effect type.", aSource->GetIdentifier());
				return INVALID_EFFECT_TYPE;
			}

			struct Effect
			{
				Effect()
				{

				}

				Effect(
					const SourceNode*		aSource)
				{
					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "type")
							m_type = SourceToEffectType(aChild);
						else if (aChild->m_name == "targets")
							m_targetFlags = SourceToTargetFlags(aChild);
						else if (aChild->m_name == "factor")
							m_factor = aChild->GetFloat();
						else if (aChild->m_name == "range")
							m_range = aChild->GetInt32();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());
					});
				}

				void
				ToStream(
					IWriter*				aWriter) const
				{
					aWriter->WritePOD(m_type);
					aWriter->WritePOD(m_targetFlags);
					aWriter->WriteFloat(m_factor);
					aWriter->WriteInt(m_range);
				}

				bool
				FromStream(
					IReader*				aReader)
				{
					if(!aReader->ReadPOD(m_type))
						return false;
					if(!aReader->ReadPOD(m_targetFlags))
						return false;
					if(!aReader->ReadFloat(m_factor))
						return false;
					if(!aReader->ReadInt(m_range))
						return false;
					return true;
				}

				// Public data
				EffectType	m_type = INVALID_EFFECT_TYPE;
				uint8_t		m_targetFlags = 0;
				float		m_factor = 0.0f;
				int32_t		m_range = 0;
			};

			DamageInputEffect()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~DamageInputEffect()
			{

			}

			// AuraEffectBase implementation
			void				OnDamageInput(
									const EntityInstance*			aSource,
									const EntityInstance*			aTarget,
									const SourceEntityInstance&		aAuraSource,
									DirectEffect::DamageType		aDamageType,
									int32_t							aDamage,
									CombatEvent::Id					aCombatEventId,
									IEventQueue*					aEventQueue,
									const IWorldView*				aWorldView,
									IResourceChangeQueue*			aResourceChangeQueue) const override;

			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "type_mask")
					{
						aChild->GetArray()->ForEachChild([&](
							const SourceNode* aFlag)
						{
							if(aFlag->IsIdentifier("all"))
							{
								m_typeMask = UINT32_MAX;
							}
							else
							{
								DirectEffect::DamageType damageType = DirectEffect::StringToDamageType(aFlag->GetIdentifier());
								TP_VERIFY(damageType != DirectEffect::INVALID_DAMAGE_TYPE, aFlag->m_debugInfo, "'%s' is not a valid damage type.", aFlag->GetIdentifier());
								m_typeMask |= 1 << (uint32_t)damageType;
							}
						});
					}
					else if (aChild->m_name == "effects")
					{
						aChild->GetArray()->ForEachChild([&](
							const SourceNode* aChild)
						{
							m_effects.push_back(Effect(aChild));
						});
					}
					else if(aChild->m_name == "aura_source_only")
					{
						m_auraSourceOnly = aChild->GetBool();
					}
					else if(aChild->m_name == "combat_log_ability")
					{
						m_combatLogAbilityId = aChild->GetId(DataType::ID_ABILITY);
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteUInt(m_typeMask);
				aStream->WriteObjects(m_effects);
				aStream->WriteBool(m_auraSourceOnly);
				aStream->WriteUInt(m_combatLogAbilityId);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadUInt(m_typeMask))
					return false;
				if (!aStream->ReadObjects(m_effects))
					return false;
				if(!aStream->ReadBool(m_auraSourceOnly))
					return false;
				if (!aStream->ReadUInt(m_combatLogAbilityId))
					return false;
				return true;
			}

			AuraEffectBase* 
			Copy() const override
			{
				DamageInputEffect* t = new DamageInputEffect();
				t->CopyBase(this);

				t->m_typeMask = m_typeMask;
				t->m_effects = m_effects;
				t->m_auraSourceOnly = m_auraSourceOnly;
				t->m_combatLogAbilityId = m_combatLogAbilityId;

				return t;
			}

			// Public data
			uint32_t				m_typeMask = 0;
			std::vector<Effect>		m_effects;			
			bool					m_auraSourceOnly = false;
			uint32_t				m_combatLogAbilityId = 0;
		};

	}

}