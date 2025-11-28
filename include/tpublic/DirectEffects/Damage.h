#pragma once

#include <tpublic/Components/AbilityModifiers.h>

#include "../CombatFunction.h"
#include "../DirectEffectBase.h"
#include "../UIntCurve.h"

namespace tpublic
{

	namespace Components
	{
		struct Auras;
	}

	namespace DirectEffects
	{

		struct Damage
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_DAMAGE;

			struct ConditionalCriticalChanceBonus
			{
				enum Type : uint8_t
				{
					INVALID_TYPE,

					TYPE_ABILITY_MODIFIER,
					TYPE_AURA
				};

				ConditionalCriticalChanceBonus()
				{

				}

				ConditionalCriticalChanceBonus(
					Type											aType,
					const SourceNode*								aSource)
					: m_type(aType)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing data annotation.");
					switch(m_type)
					{
					case TYPE_ABILITY_MODIFIER: m_id = aSource->m_annotation->GetId(DataType::ID_ABILITY_MODIFIER); break;
					case TYPE_AURA:				m_id = aSource->m_annotation->GetId(DataType::ID_AURA); break;
					default:					assert(false);
					}

					m_percent = aSource->GetFloat();
				}

				void	
				ToStream(
					IWriter*										aStream) const
				{
					aStream->WritePOD(m_type);
					aStream->WriteUInt(m_id);
					aStream->WriteFloat(m_percent);
				}

				bool
				FromStream(
					IReader*										aStream)
				{
					if(!aStream->ReadPOD(m_type))
						return false;
					if(!aStream->ReadUInt(m_id))
						return false;
					if(!aStream->ReadFloat(m_percent))
						return false;
					return true;
				}

				// Public data
				Type					m_type = INVALID_TYPE;
				uint32_t				m_id = 0;
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
								const Manifest*						aManifest,
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
			float										m_threatMultiplier = 1.0f;
			float										m_spread = 0.0f;
			uint32_t									m_resolveCancelAuraId = 0;
			bool										m_direct = true;

			DirectEffect::DamageType	_GetDamageType(
											const EntityInstance*				aEntityInstance,
											const AbilityModifierList*			aAbilityModifierList,
											uint32_t							aAbilityId) const;
			float						_GetCriticalChanceBonus(
											const Components::Auras*			aAuras,
											const Components::AbilityModifiers*	aAbilityModifiers) const;
			float						_GetDamageModifier(
											const Components::AbilityModifiers*	aAbilityModifiers) const;
		};

	}

}