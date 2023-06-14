#pragma once

#include "IReader.h"
#include "IWriter.h"

namespace tpublic
{

	class PlayerTag
	{
	public:
		enum Type : uint8_t
		{
			TYPE_NONE,
			TYPE_INDIVIDUAL,
			TYPE_GROUP
		};

		void
		Clear()
		{
			m_type = TYPE_NONE;
		}

		void
		SetEntityInstanceId(
			uint32_t		aEntityInstanceId)
		{
			m_type = TYPE_INDIVIDUAL;
			m_value = (uint64_t)aEntityInstanceId;
		}

		void
		SetGroupId(
			uint64_t		aGroupId)
		{
			m_type = TYPE_GROUP;
			m_value = aGroupId;
		}

		void
		ToStream(
			IWriter*		aWriter) const
		{
			aWriter->WritePOD(m_type);
			if(IsSet())
				aWriter->WriteUInt(m_value);
		}

		bool
		FromStream(
			IReader*		aReader)
		{
			if(!aReader->ReadPOD(m_type))
				return false;
			if(IsSet())
			{
				if(!aReader->ReadUInt(m_value))
					return false;
			}
			return true;
		}

		// Data access
		Type		GetType() const { return m_type; }
		bool		IsSet() const { return m_type != TYPE_NONE; }
		bool		IsIndividual() const { return m_type == TYPE_INDIVIDUAL; }
		bool		IsGroup() const { return m_type == TYPE_GROUP; }
		uint32_t	GetEntityInstanceId() const { assert(IsIndividual()); return (uint32_t)m_value; }
		uint64_t	GetGroupId() const { assert(IsGroup()); return m_value; }

	private:

		Type				m_type = TYPE_NONE;
		uint64_t			m_value = 0;
	};

}