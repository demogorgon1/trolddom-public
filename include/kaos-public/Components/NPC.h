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
			static const uint8_t FLAGS = FLAG_SHARED_OWNER | FLAG_SHARED_OTHERS;

			NPC()
				: ComponentBase(ID, FLAGS)
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

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInts(m_abilities);
				m_stats.ToStream(aStream);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadUInts(m_abilities))
					return false;
				if(!m_stats.FromStream(aStream))
					return false;
				return true;
			}

			// Public data			
			std::vector<uint32_t>	m_abilities;
			Stat::Collection		m_stats;
		};
	}

}