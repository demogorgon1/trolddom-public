#pragma once

#include "Component.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace kaos_public
{

	class ComponentBase
	{
	public:
		ComponentBase(
			uint32_t	aComponentId)
			: m_componentId(aComponentId)
		{

		}

		virtual 
		~ComponentBase()
		{

		}

		// Virtual methods
		virtual void	FromSource(
							const Parser::Node*		/*aSource*/) { assert(false); }
		virtual void	ToStream(
							IWriter*				/*aStream*/) const { assert(false); }
		virtual bool	FromStream(
							IReader*				/*aStream*/) { assert(false); return true; }

		// Data access
		uint32_t		GetComponentId() const { return m_componentId; }

	private:

		uint32_t		m_componentId;
	};

}