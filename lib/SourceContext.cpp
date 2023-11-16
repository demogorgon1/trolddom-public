#include "Pcheader.h"

#include <tpublic/AuraEffectFactory.h>
#include <tpublic/ComponentManager.h>
#include <tpublic/DirectEffectFactory.h>
#include <tpublic/MapGeneratorFactory.h>
#include <tpublic/PersistentIdTable.h>
#include <tpublic/SourceContext.h>

namespace tpublic
{

	SourceContext::SourceContext()
	{
		m_auraEffectFactory = std::make_unique<AuraEffectFactory>();
		m_componentManager = std::make_unique<ComponentManager>();
		m_directEffectFactory = std::make_unique<DirectEffectFactory>();
		m_mapGeneratorFactory = std::make_unique<MapGeneratorFactory>();
		m_persistentIdTable = std::make_unique<PersistentIdTable>();
	}

}