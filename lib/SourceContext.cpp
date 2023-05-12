#include "Pcheader.h"

#include <tpublic/AuraEffectFactory.h>
#include <tpublic/ComponentFactory.h>
#include <tpublic/DirectEffectFactory.h>
#include <tpublic/PersistentIdTable.h>
#include <tpublic/SourceContext.h>

namespace tpublic
{

	SourceContext::SourceContext()
	{
		m_auraEffectFactory = std::make_unique<AuraEffectFactory>();
		m_componentFactory = std::make_unique<ComponentFactory>();
		m_directEffectFactory = std::make_unique<DirectEffectFactory>();
		m_persistentIdTable = std::make_unique<PersistentIdTable>();
	}

}