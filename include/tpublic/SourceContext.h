#pragma once

namespace tpublic
{

	class AuraEffectFactory;
	class ComponentFactory;
	class DirectEffectFactory;
	class PersistentIdTable;
	
	struct SourceContext
	{
		SourceContext();

		// Public data
		std::unique_ptr<AuraEffectFactory>		m_auraEffectFactory;
		std::unique_ptr<ComponentFactory>		m_componentFactory;
		std::unique_ptr<DirectEffectFactory>	m_directEffectFactory;
		std::unique_ptr<PersistentIdTable>		m_persistentIdTable;
	};

}