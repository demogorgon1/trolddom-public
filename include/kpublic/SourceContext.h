#pragma once

namespace kpublic
{

	class ComponentFactory;
	class DirectEffectFactory;
	class PersistentIdTable;
	
	struct SourceContext
	{
		SourceContext();

		// Public data
		std::unique_ptr<ComponentFactory>		m_componentFactory;
		std::unique_ptr<DirectEffectFactory>			m_effectFactory;
		std::unique_ptr<PersistentIdTable>		m_persistentIdTable;
	};

}