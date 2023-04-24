#include "Pcheader.h"

#include <kpublic/AuraEffectFactory.h>
#include <kpublic/ComponentFactory.h>
#include <kpublic/DirectEffectFactory.h>
#include <kpublic/PersistentIdTable.h>
#include <kpublic/SourceContext.h>

namespace kpublic
{

	SourceContext::SourceContext()
	{
		m_auraEffectFactory = std::make_unique<AuraEffectFactory>();
		m_componentFactory = std::make_unique<ComponentFactory>();
		m_directEffectFactory = std::make_unique<DirectEffectFactory>();
		m_persistentIdTable = std::make_unique<PersistentIdTable>();
	}

}