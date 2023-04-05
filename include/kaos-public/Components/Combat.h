#pragma once

#include "../ComponentBase.h"

namespace kaos_public
{

	namespace Components
	{

		struct Combat
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_COMBAT;

			Combat()
				: ComponentBase(ID)
			{

			}

			virtual 
			~Combat()
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
					if (aChild->m_name == "max_health")
						m_maxHealth = aChild->GetUInt32();
					else if (aChild->m_name == "current_health")
						m_currentHealth = aChild->GetUInt32();
					else
						KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			void	
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteUInt(m_targetEntityInstanceId);
				aStream->WriteUInt(m_currentHealth);
				aStream->WriteUInt(m_maxHealth);
			}
			
			bool	
			FromStream(
				IReader*				aStream) override
			{
				if (!aStream->ReadUInt(m_targetEntityInstanceId))
					return false;
				if (!aStream->ReadUInt(m_currentHealth))
					return false;
				if (!aStream->ReadUInt(m_maxHealth))
					return false;
				return true;
			}

			// Public data
			uint32_t		m_targetEntityInstanceId = 0;

			uint32_t		m_currentHealth = 1;
			uint32_t		m_maxHealth = 1;
		};

	}

}