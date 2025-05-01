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
			TYPE_NONE,			// NPC: Not tagged, loot: can't be looted by anyone
			TYPE_CHARACTER,		// NPC: tagged by character, loot: can be looted by character
			TYPE_GROUP,			// NPC: tagged by group
			TYPE_ANYONE,		// Loot: can be looted by anyone
			TYPE_MASTER			// Loot: can be assigned by master looter,
		};

		PlayerTag(
			Type			aType = TYPE_NONE,
			uint64_t		aValue = 0)
			: m_type(aType)
			, m_value(aValue)
			, m_characterLevel(0)
		{

		}

		void
		Clear()
		{
			m_type = TYPE_NONE;
			m_value = 0;
			m_characterLevel = 0;
		}

		void
		SetCharacter(
			uint32_t		aCharacterId,
			uint32_t		aCharacterLevel)
		{
			m_type = TYPE_CHARACTER;
			m_value = (uint64_t)aCharacterId;
			m_characterLevel = aCharacterLevel;
		}

		void
		SetGroupId(
			uint64_t		aGroupId)
		{
			m_type = TYPE_GROUP;
			m_value = aGroupId;
			m_characterLevel = 0;
		}

		void
		ToStream(
			IWriter*		aWriter) const
		{
			aWriter->WritePOD(m_type);
			if(IsSet())
				aWriter->WriteUInt(m_value);
			if(m_type == TYPE_CHARACTER)
				aWriter->WriteUInt(m_characterLevel);
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
			if(m_type == TYPE_CHARACTER)
			{
				if(!aReader->ReadUInt(m_characterLevel))
					return false;
			}
			return true;
		}

		bool
		operator ==(
			const PlayerTag&	aOther) const
		{
			// No need to compare character level
			return m_type == aOther.m_type && m_value == aOther.m_value;
		}

		bool
		operator !=(
			const PlayerTag&	aOther) const
		{
			return !this->operator==(aOther);
		}

		// Data access
		Type		GetType() const { return m_type; }
		bool		IsSet() const { return m_type != TYPE_NONE; }
		bool		IsCharacter() const { return m_type == TYPE_CHARACTER; }
		bool		IsGroup() const { return m_type == TYPE_GROUP; }
		bool		IsAnyone() const { return m_type == TYPE_ANYONE; }
		bool		IsMaster() const { return m_type == TYPE_MASTER; }
		uint32_t	GetCharacterId() const { assert(IsCharacter()); return (uint32_t)m_value; }
		uint32_t	GetCharacterLevel() const { assert(IsCharacter()); return m_characterLevel; }
		uint64_t	GetGroupId() const { assert(IsGroup()); return m_value; }

	private:

		Type				m_type;
		uint64_t			m_value;
		uint32_t			m_characterLevel;
	};

}
