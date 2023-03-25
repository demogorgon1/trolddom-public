#include "Pcheader.h"

#include <kaos-public/ComponentFactory.h>
#include <kaos-public/EffectFactory.h>
#include <kaos-public/PersistentIdTable.h>
#include <kaos-public/SourceContext.h>

namespace kaos_public
{

	SourceContext::SourceContext()
	{
		m_componentFactory = std::make_unique<ComponentFactory>();
		m_effectFactory = std::make_unique<EffectFactory>();
		m_persistentIdTable = std::make_unique<PersistentIdTable>();
	}

}