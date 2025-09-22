#pragma once

#include "../AuraEffectBase.h"
#include "../DirectEffect.h"

namespace tpublic
{

	class Manifest;

	namespace AuraEffects
	{

		struct CastModifier
			: public AuraEffectBase
		{
			static const AuraEffect::Id ID = AuraEffect::ID_CAST_MODIFIER;
			
			CastModifier()
				: AuraEffectBase(ID)
			{

			}

			virtual 
			~CastModifier()
			{

			}

			// AuraEffectBase implementation
			void				FromSource(
									const SourceNode*				aSource) override;
			void				ToStream(
									IWriter*						aStream) const override;
			bool				FromStream(
									IReader*						aStream) override;
			AuraEffectBase*		Copy() const override;
			bool				UpdateCastTime(
									const Manifest*					aManifest,
									uint32_t						aAbilityId,
									uint32_t&						aCharges,
									int32_t&						aCastTime) override;

			// Public data
			uint32_t				m_abilityFlags = 0;
			float					m_castTimeModifier = 1.0f;
			bool					m_useCharge = false;
			std::vector<uint32_t>	m_abilityIds;
		};

	}

}