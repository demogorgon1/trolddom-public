#pragma once

#include "SourceNode.h"

namespace tpublic
{

	class EntityInstance;

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
			m_abilityId = aSource->GetId(DataType::ID_ABILITY);
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

		const EntityInstance*
		ResolveTarget(
			const EntityInstance*	aSelf,
			const EntityInstance*	aTarget) const
		{
			switch(m_target)
			{
			case TARGET_SELF:		return aSelf;
			case TARGET_TARGET:		return aTarget;
			default:				break;
			}
			return NULL;
		}

		uint32_t
		ResolveTargetEntityInstanceId(
			uint32_t				aSelfEntityInstanceId,
			uint32_t				aTargetEntityInstanceId) const
		{
			switch(m_target)
			{
			case TARGET_SELF:		return aSelfEntityInstanceId;
			case TARGET_TARGET:		return aTargetEntityInstanceId;
			default:				break;
			}
			return 0;
		}

		// Public data
		uint32_t		m_abilityId = 0;
		Target			m_target = INVALID_TARGET;		
	};

}