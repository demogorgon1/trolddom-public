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
		enum Flag : uint8_t
		{
			FLAG_SHARED_OWNER	= 0x01,
			FLAG_SHARED_OTHERS	= 0x02,
			FLAG_PERSISTENT		= 0x04,
			FLAG_PLAYER_ONLY	= 0x08
		};

		ComponentBase(
			uint32_t	aComponentId,
			uint8_t		aFlags)
			: m_componentId(aComponentId)
			, m_flags(aFlags)
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
		uint8_t			GetFlags() const { return m_flags; }

	private:

		uint32_t		m_componentId;
		uint8_t			m_flags;
	};

}