#pragma once

#include "../DataBase.h"
#include "../Requirement.h"

namespace tpublic
{

	namespace Data
	{

		struct AbilityList
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_ABILITY_LIST;
			static const bool TAGGED = false;

			struct Entry
			{
				Entry()
				{

				}

				Entry(
					const SourceNode*	aSource)
				{
					m_abilityId = aSource->m_sourceContext->m_persistentIdTable->GetId(aSource->m_debugInfo, DataType::ID_ABILITY, aSource->m_name.c_str());

					aSource->GetObject()->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_tag == "requirement")
							m_requirements.push_back(Requirement(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_abilityId);
					aWriter->WriteObjects(m_requirements);
				}

				bool
				FromStream(
					IReader*			aReader)
				{
					if(!aReader->ReadUInt(m_abilityId))
						return false;
					if(!aReader->ReadObjects(m_requirements))
						return false;
					return true;
				}

				// Public data
				uint32_t					m_abilityId = 0;
				std::vector<Requirement>	m_requirements;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aNode) override
			{	
				aNode->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if(aChild->m_tag == "ability")
							m_entries.push_back(std::make_unique<Entry>(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteObjectPointers(m_entries);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadObjectPointers(m_entries))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Entry>> m_entries;
		};

	}

}