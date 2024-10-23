#pragma once

#include <tpublic/Components/AbilityModifiers.h>

#include "../CombatFunction.h"
#include "../DirectEffectBase.h"
#include "../UIntCurve.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct Damage
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_DAMAGE;

			struct ConditionalCriticalChanceBonus
			{
				ConditionalCriticalChanceBonus()
				{

				}

				ConditionalCriticalChanceBonus(
					const SourceNode*								aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing ability modifier annotation.");
					m_abilityModifierId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY_MODIFIER, aSource->m_annotation->GetIdentifier());
					m_percent = aSource->GetFloat();
				}

				void	
				ToStream(
					IWriter*										aStream) const
				{
					aStream->WriteUInt(m_abilityModifierId);
					aStream->WriteFloat(m_percent);
				}

				bool
				FromStream(
					IReader*										aStream)
				{
					if(!aStream->ReadUInt(m_abilityModifierId))
						return false;
					if(!aStream->ReadFloat(m_percent))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_abilityModifierId = 0;
				float					m_percent = 0.0f;
			};

			Damage()
			{

			}

			virtual 
			~Damage()
			{

			}

			// DirectEffectBase implementation
			void			FromSource(
								const SourceNode*					aSource) override;
			void			ToStream(
								IWriter*							aStream) const override;
			bool			FromStream(
								IReader*							aStream) override;
			Result			Resolve(
								int32_t								aTick,
								std::mt19937&						aRandom,
								const Manifest*						aManifest,
								CombatEvent::Id						aId,
								uint32_t							aAbilityId,
								const SourceEntityInstance&			aSourceEntityInstance,
								EntityInstance*						aSource,
								EntityInstance*						aTarget,
								const Vec2&							aAOETarget,
								const ItemInstanceReference&		aItem,
								IResourceChangeQueue*				aResourceChangeQueue,
								IAuraEventQueue*					aAuraEventQueue,
								IEventQueue*						aEventQueue,
								const IWorldView*					aWorldView) override;
			bool			CalculateToolTipDamage(
								const EntityInstance*				aEntityInstance,
								const AbilityModifierList*			aAbilityModifierList,
								uint32_t							aAbilityId,
								UIntRange&							aOutDamage,
								DirectEffect::DamageType&			aOutDamageType) const override;

			// Public data
			DirectEffect::DamageType					m_damageType = DirectEffect::DAMAGE_TYPE_PHYSICAL;

			CombatFunction								m_function;
			std::vector<ConditionalCriticalChanceBonus>	m_conditionalCriticalChanceBonuses;
			std::unordered_map<uint32_t, float>			m_abilityModifierMultipliers;

			DirectEffect::DamageType	_GetDamageType(
											const EntityInstance*				aEntityInstance,
											const AbilityModifierList*			aAbilityModifierList,
											uint32_t							aAbilityId) const;
			float						_GetCriticalChanceBonus(
											const Components::AbilityModifiers*	aAbilityModifiers) const;
			float						_GetDamageModifier(
											const Components::AbilityModifiers*	aAbilityModifiers) const;
		};

	}

}