#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"
#include "SoundEffect.h"

namespace tpublic
{

	class DefaultSoundEffects
	{
	public:		
		void
		FromSource(
			const SourceNode*			aSource)
		{
			aSource->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "melee")
					m_melee.FromSource(aChild);
				else if (aChild->m_name == "offensive_spell")
					m_offensiveSpell.FromSource(aChild);
				else if (aChild->m_name == "misc")
					m_misc.FromSource(aChild);
				else if (aChild->m_name == "consumable")
					m_consumableSoundId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SOUND, aChild->GetIdentifier());
				else if (aChild->m_name == "level_up")
					m_levelUpSoundId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_SOUND, aChild->GetIdentifier());
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void
		ToStream(
			IWriter*						aStream) const 
		{
			m_melee.ToStream(aStream);
			m_offensiveSpell.ToStream(aStream);
			m_misc.ToStream(aStream);
			aStream->WriteUInt(m_consumableSoundId);
			aStream->WriteUInt(m_levelUpSoundId);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if (!m_melee.FromStream(aStream))
				return false;
			if (!m_offensiveSpell.FromStream(aStream))
				return false;
			if (!m_misc.FromStream(aStream))
				return false;
			if (!aStream->ReadUInt(m_consumableSoundId))
				return false;
			if (!aStream->ReadUInt(m_levelUpSoundId))
				return false;
			return true;
		}
				
		// Public data
		SoundEffect::Collection	m_melee;
		SoundEffect::Collection	m_offensiveSpell;
		SoundEffect::Collection	m_misc;

		uint32_t				m_consumableSoundId = 0;
		uint32_t				m_levelUpSoundId = 0;
	};

}
