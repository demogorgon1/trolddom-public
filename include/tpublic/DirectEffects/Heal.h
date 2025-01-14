#pragma once

#include "../Components/AbilityModifiers.h"

#include "../CombatFunction.h"
#include "../DirectEffectBase.h"

namespace tpublic
{

	namespace DirectEffects
	{

		struct Heal
			: public DirectEffectBase
		{
			static const DirectEffect::Id ID = DirectEffect::ID_HEAL;

			struct ConditionalCriticalChanceBonus
			{
				ConditionalCriticalChanceBonus()
				{

				}

				ConditionalCriticalChanceBonus(
					const SourceNode*								aSource)
				{
					TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing ability modifier annotation.");
					m_abilityModifierId = aSource->m_annotation->GetId(DataType::ID_ABILITY_MODIFIER);
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

			Heal()
			{

			}

			virtual 
			~Heal()
			{

			}

			// EffectBase implementation
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
			bool			CalculateToolTipHeal(
								const EntityInstance*				aEntityInstance,
								const AbilityModifierList*			aAbilityModifierList,
								uint32_t							aAbilityId,
								UIntRange&							aOutHeal) const override;

			// Public data
			CombatFunction								m_function;
			bool										m_maxHealthPercentage = false;
			std::vector<ConditionalCriticalChanceBonus>	m_conditionalCriticalChanceBonuses;

			float			_GetCriticalChanceBonus(
								const Components::AbilityModifiers*	aAbilityModifiers) const;
		};

	}

}