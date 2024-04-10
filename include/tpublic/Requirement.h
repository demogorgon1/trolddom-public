#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class Requirement
	{
	public:
		enum Target : uint8_t
		{
			INVALID_TARGET,

			TARGET_TARGET,
			TARGET_SELF
		};

		enum Type : uint8_t
		{
			INVALID_TYPE,

			TYPE_MUST_HAVE_AURA,
			TYPE_MUST_NOT_HAVE_AURA,
			TYPE_MUST_HAVE_COMPLETED_QUEST,
			TYPE_MUST_NOT_HAVE_COMPLETED_QUEST,
			TYPE_MUST_HAVE_ACTIVE_QUEST,
			TYPE_MUST_NOT_HAVE_ACTIVE_QUEST
		};

		static DataType::Id
		GetDataType(
			Type					aType)
		{
			switch (aType)
			{
			case TYPE_MUST_HAVE_AURA:
			case TYPE_MUST_NOT_HAVE_AURA:
				return DataType::ID_AURA;

			case TYPE_MUST_HAVE_COMPLETED_QUEST:
			case TYPE_MUST_NOT_HAVE_COMPLETED_QUEST:
			case TYPE_MUST_HAVE_ACTIVE_QUEST:
			case TYPE_MUST_NOT_HAVE_ACTIVE_QUEST:
				return DataType::ID_QUEST;

			default:
				break;
			}
			assert(false);
			return DataType::INVALID_ID;
		}

		Requirement()
		{

		}

		Requirement(
			const SourceNode* aSource)
		{
			if (aSource->m_name == "target")
				m_target = TARGET_TARGET;
			else if (aSource->m_name == "self")
				m_target = TARGET_SELF;
			else
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid target.", aSource->m_name.c_str());

			TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing type annotation.");
			std::string_view typeString(aSource->m_annotation->GetIdentifier());
			if (typeString == "must_have_aura")
				m_type = TYPE_MUST_HAVE_AURA;
			else if (typeString == "must_not_have_aura")
				m_type = TYPE_MUST_NOT_HAVE_AURA;
			else if (typeString == "must_have_completed_quest")
				m_type = TYPE_MUST_HAVE_COMPLETED_QUEST;
			else if (typeString == "must_not_have_completed_quest")
				m_type = TYPE_MUST_NOT_HAVE_COMPLETED_QUEST;
			else if (typeString == "must_have_active_quest")
				m_type = TYPE_MUST_HAVE_ACTIVE_QUEST;
			else if (typeString == "must_not_have_active_quest")
				m_type = TYPE_MUST_NOT_HAVE_ACTIVE_QUEST;
			else
				TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid type.", aSource->m_annotation->GetIdentifier());

			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if (aChild->m_name == "id")
					m_id = aChild->m_sourceContext->m_persistentIdTable->GetId(GetDataType(m_type), aChild->GetIdentifier());
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void
		ToStream(
			IWriter* aWriter) const
		{
			aWriter->WritePOD(m_target);
			aWriter->WritePOD(m_type);
			aWriter->WriteUInt(m_id);
		}

		bool
		FromStream(
			IReader* aReader)
		{
			if (!aReader->ReadPOD(m_target))
				return false;
			if (!aReader->ReadPOD(m_type))
				return false;
			if (!aReader->ReadUInt(m_id))
				return false;
			return true;
		}

		// Public data
		Target								m_target = INVALID_TARGET;
		Type								m_type = INVALID_TYPE;
		uint32_t							m_id = 0;
	};

}