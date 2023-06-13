#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{

	class LevelXPRequirements
	{
	public:
		LevelXPRequirements()
		{

		}

		void
		FromSource(
			const Parser::Node*	aSource)
		{
			aSource->GetArray()->ForEachChild([&](
				const Parser::Node* aChild)
			{
				TP_VERIFY(aChild->m_type == Parser::Node::TYPE_ARRAY && aChild->m_children.size() == 2, aChild->m_debugInfo, "Not a level-xp pair.");
				uint32_t level = aChild->m_children[0]->GetUInt32();
				uint32_t xp = aChild->m_children[1]->GetUInt32();

				if(level >= m_xpPerLevel.size())
					m_xpPerLevel.resize(level + 1);

				m_xpPerLevel[level] = xp;
			});
		}

		void
		ToStream(
			IWriter*			aStream) const 
		{
			aStream->WriteUInts(m_xpPerLevel);
		}

		bool
		FromStream(
			IReader*			aStream) 
		{
			if(!aStream->ReadUInts(m_xpPerLevel))
				return false;
			return true;
		}
		
		uint32_t
		GetXPRequiredForLevel(
			uint32_t			aLevel) const
		{
			if(aLevel >= m_xpPerLevel.size())
				return 0;
			return m_xpPerLevel[aLevel];
		}
		
		// Public data
		std::vector<uint32_t>		m_xpPerLevel;
	};

}