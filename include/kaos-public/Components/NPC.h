#pragma once

#include "../Component.h"
#include "../Stat.h"

namespace kaos_public
{

	namespace Components
	{

		struct NPC
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_NPC;

			NPC()
				: ComponentBase(ID)
			{

			}

			virtual  
			~NPC()
			{

			}

			// ComponentBase implementation
			void	
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if (aChild->m_name == "abilities")
						aChild->GetIdArray(DataType::ID_ABILITY, m_abilities);
					else if (aChild->m_name == "stats")
						m_stats.FromSource(aChild);
					else
						KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			// Public data			
			std::vector<uint32_t>	m_abilities;
			Stat::Collection		m_stats;
		};
	}

}