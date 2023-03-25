#pragma once

namespace kaos_public
{

	class ComponentFactory;
	class EffectFactory;
	class PersistentIdTable;
	
	struct SourceContext
	{
		SourceContext();

		// Public data
		std::unique_ptr<ComponentFactory>		m_componentFactory;
		std::unique_ptr<EffectFactory>			m_effectFactory;
		std::unique_ptr<PersistentIdTable>		m_persistentIdTable;
	};

}