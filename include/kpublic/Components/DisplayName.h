#pragma once

#include "Component.h"
#include "Vec2.h"

namespace kpublic
{

	namespace Components
	{

		struct DisplayName
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_DISPLAY_NAME;
			static const uint8_t FLAGS = FLAG_PRIVATE | FLAG_PUBLIC;

			DisplayName()
				: ComponentBase(ID, FLAGS)
			{

			}

			virtual 
			~DisplayName()
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
					if (aChild->m_name == "string")
						m_string = aChild->GetString();
					else
						KP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteString(m_string);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadString(m_string))
					return false;
				return true;
			}

			// Public data
			std::string		m_string;
		};
	}

}