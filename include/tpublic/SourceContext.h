#pragma once

namespace tpublic
{

	class AuraEffectFactory;
	class ComponentManager;
	class DirectEffectFactory;
	class MapGeneratorFactory;
	class PersistentIdTable;
	
	struct SourceContext
	{
		SourceContext();

		// Public data
		std::unique_ptr<AuraEffectFactory>		m_auraEffectFactory;
		std::unique_ptr<ComponentManager>		m_componentManager;
		std::unique_ptr<DirectEffectFactory>	m_directEffectFactory;
		std::unique_ptr<MapGeneratorFactory>	m_mapGeneratorFactory;
		std::unique_ptr<PersistentIdTable>		m_persistentIdTable;
	};

}