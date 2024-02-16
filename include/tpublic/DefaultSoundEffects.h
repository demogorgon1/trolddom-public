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
				else if (aChild->m_name == "ranged")
					m_ranged.FromSource(aChild);
				else if (aChild->m_name == "spell")
					m_spell.FromSource(aChild);
				else if (aChild->m_name == "misc")
					m_misc.FromSource(aChild);
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
			});
		}

		void
		ToStream(
			IWriter*						aStream) const 
		{
			m_melee.ToStream(aStream);
			m_ranged.ToStream(aStream);
			m_spell.ToStream(aStream);
			m_misc.ToStream(aStream);
		}

		bool
		FromStream(
			IReader*						aStream) 
		{
			if (!m_melee.FromStream(aStream))
				return false;
			if (!m_ranged.FromStream(aStream))
				return false;
			if (!m_spell.FromStream(aStream))
				return false;
			if (!m_misc.FromStream(aStream))
				return false;
			return true;
		}
				
		// Public data
		SoundEffect::Collection	m_melee;
		SoundEffect::Collection	m_ranged;
		SoundEffect::Collection	m_spell;
		SoundEffect::Collection	m_misc;
	};

}
