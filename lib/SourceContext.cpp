#include "Pcheader.h"

#include <kpublic/ComponentFactory.h>
#include <kpublic/EffectFactory.h>
#include <kpublic/PersistentIdTable.h>
#include <kpublic/SourceContext.h>

namespace kpublic
{

	SourceContext::SourceContext()
	{
		m_componentFactory = std::make_unique<ComponentFactory>();
		m_effectFactory = std::make_unique<EffectFactory>();
		m_persistentIdTable = std::make_unique<PersistentIdTable>();
	}

}