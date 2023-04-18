#pragma once

#include "Component.h"
#include "DataErrorHandling.h"
#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace kpublic
{

	class ComponentBase
	{
	public:
		enum Flag : uint8_t
		{
			FLAG_PRIVATE		= 0x01,
			FLAG_PUBLIC			= 0x02,
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

		template <typename _T>
		const _T*
		Cast() const
		{
			KP_CHECK(m_componentId == _T::ID, "Component type mismatch.");
			return (const _T*)this;
		}

		template <typename _T>
		_T*
		Cast()
		{
			KP_CHECK(m_componentId == _T::ID, "Component type mismatch.");
			return (_T*)this;
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