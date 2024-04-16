#pragma once

#include "SourceNode.h"

namespace tpublic
{

	class SecondaryAbility
	{
	public:	
		enum Target : uint8_t
		{
			INVALID_TARGET,

			TARGET_TARGET,
			TARGET_SELF
		};	

		static Target
		SourceToTarget(
			const SourceNode*		aSource)
		{
			std::string_view t(aSource->GetIdentifier());
			if(t == "target")
				return TARGET_TARGET;
			else if(t == "self")
				return TARGET_SELF;
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid target.", aSource->GetIdentifier());
			return INVALID_TARGET;
		}
	
		SecondaryAbility()
		{

		}

		SecondaryAbility(
			const SourceNode*		aSource)
		{
			TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing target annotation.");
			m_target = SourceToTarget(aSource->m_annotation.get());
			m_abilityId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_ABILITY, aSource->GetIdentifier());
		}

		void
		ToStream(
			IWriter*				aWriter) const
		{
			aWriter->WriteUInt(m_abilityId);
			aWriter->WritePOD(m_target);
		}

		bool
		FromStream(
			IReader*				aReader)
		{
			if (!aReader->ReadUInt(m_abilityId))
				return false;
			if (!aReader->ReadPOD(m_target))
				return false;
			return true;
		}

		// Public data
		uint32_t		m_abilityId = 0;
		Target			m_target = INVALID_TARGET;		
	};

}