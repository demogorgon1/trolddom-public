#pragma once

#include "DirectEffect.h"
#include "IReader.h"
#include "IWriter.h"
#include "Requirement.h"

namespace tpublic
{

	class EntityInstance;
	class Manifest;

	class ToolTipMultiplier
	{
	public:
		enum Type : uint8_t
		{
			INVALID_TYPE,

			TYPE_DAMAGE_OUTPUT,
			TYPE_HEAL_OUTPUT
		};

		static float	Resolve(
							const std::vector<ToolTipMultiplier>&	aToolTipMultipliers,
							Type									aType,
							uint32_t								aAbilityId,
							DirectEffect::DamageType				aDamageType,
							const Manifest*							aManifest,
							const EntityInstance*					aEntityInstance);

		void
		ToStream(
			IWriter*		aWriter) const
		{
			aWriter->WritePOD(m_type);
			aWriter->WriteUInts(m_abilityIds);
			aWriter->WriteFloat(m_multiplier);
			aWriter->WriteUInt(m_typeMask);
			aWriter->WriteObjects(m_requirements);
		}

		bool
		FromStream(
			IReader*		aReader)
		{
			if (!aReader->ReadPOD(m_type))
				return false;
			if (!aReader->ReadUInts(m_abilityIds))
				return false;
			if (!aReader->ReadFloat(m_multiplier))
				return false;
			if (!aReader->ReadUInt(m_typeMask))
				return false;
			if(!aReader->ReadObjects(m_requirements))
				return false;
			return true;
		}

		// Public data
		Type							m_type = INVALID_TYPE;
		std::vector<uint32_t>			m_abilityIds;
		std::vector<Requirement>		m_requirements;
		float							m_multiplier = 0.0f;
		uint32_t						m_typeMask = 0;
	};

}