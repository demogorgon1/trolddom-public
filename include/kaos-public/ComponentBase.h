#pragma once

#include "Component.h"
#include "Parser.h"

namespace kaos_public
{

	class ComponentBase
	{
	public:
		ComponentBase()
		{

		}

		virtual 
		~ComponentBase()
		{

		}

		// Virtual methods
		virtual void	FromSource(
							const Parser::Node*		/*aSource*/) { assert(false); }

	private:
	};

}