#pragma once

#include "Component.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{

	class PlayerComponents
	{
	public:
		PlayerComponents()
		{

		}

		void
		FromSource(
			const Parser::Node*	aSource)
		{
			aSource->GetArray()->ForEachChild([&](
				const Parser::Node* aChild)
			{
				uint32_t componentId = Component::StringToId(aChild->GetIdentifier());
				TP_VERIFY(componentId != 0, aChild->m_debugInfo, "'%s' is not a valid component.", aChild->GetIdentifier());
				m_components.push_back(componentId);
			});
		}

		void
		ToStream(
			IWriter*			aStream) const 
		{
			aStream->WriteUInts(m_components);
		}

		bool
		FromStream(
			IReader*			aStream) 
		{
			if(!aStream->ReadUInts(m_components))
				return false;
			return true;
		}
		
		// Public data
		std::vector<uint32_t>		m_components;
	};

}