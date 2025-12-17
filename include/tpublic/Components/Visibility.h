#pragma once

#include "../ComponentBase.h"
#include "../Requirement.h"

namespace tpublic
{

	namespace Components
	{

		struct Visibility
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_VISIBILITY;
			static const uint8_t FLAGS = 0;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;
			static const Replication REPLICATION = REPLICATION_PUBLIC;

			struct Requirements
			{
				void
				FromSource(
					const SourceNode*	aSource)
				{
					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_tag == "requirement")						
							m_entries.push_back(Requirement(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteObjects(m_entries);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadObjects(m_entries))
						return false;
					return true;
				}

				// Public data
				std::vector<Requirement>	m_entries;
			};

			enum Field : uint32_t
			{
				FIELD_REQUIREMENTS
			};

			static void
			CreateSchema(
				ComponentSchema*		aSchema)
			{
				aSchema->DefineCustomObject<Requirements>(FIELD_REQUIREMENTS, "requirements", offsetof(Visibility, m_requirements));
			}

			void
			Reset()
			{
				m_requirements.m_entries.clear();
			}

			// Public data
			Requirements					m_requirements;			
		};
	}

}