#pragma once

#include "System.h"

namespace kaos_public
{
	
	class EntityInstance;
	class Manifest;

	class SystemBase
	{
	public:
		SystemBase(
			const Manifest*	aManifest)
			: m_manifest(aManifest)
		{

		}

		virtual			
		~SystemBase() 
		{

		}

		// Virtual methods
		virtual void	Init(
							EntityInstance*		/*aEntity*/) { }
		virtual void	Update(
							EntityInstance*		/*aEntity*/) { }

		// Data access
		const Manifest*	GetManifest() const { return m_manifest; }

	private:

		const Manifest*		m_manifest;
	};

}