#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct DialogueRoot
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_DIALOGUE_ROOT;

			struct Entry
			{
				Entry()
				{

				}

				Entry(
					const SourceNode*	aSource)
				{
					m_dialogueScreenId = aSource->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_DIALOGUE_SCREEN, aSource->m_name.c_str());

					aSource->ForEachChild([&](
						const SourceNode* aChild)
					{
						if(aChild->m_name == "conditions")
							aChild->GetIdArray(DataType::ID_EXPRESSION, m_conditionExpressionIds);
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					});
				}

				void
				ToStream(
					IWriter*			aWriter) const
				{
					aWriter->WriteUInt(m_dialogueScreenId);
					aWriter->WriteUInts(m_conditionExpressionIds);
				}

				bool
				FromStream(
					IReader*			aReader) 
				{
					if(!aReader->ReadUInt(m_dialogueScreenId))
						return false;
					if(!aReader->ReadUInts(m_conditionExpressionIds))
						return false;
					return true;
				}

				// Public data
				uint32_t				m_dialogueScreenId = 0;
				std::vector<uint32_t>	m_conditionExpressionIds;
			};

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					m_entries.push_back(std::make_unique<Entry>(aChild));
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WriteObjectPointers(m_entries);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if(!aStream->ReadObjectPointers(m_entries))
					return false;
				return true;
			}

			// Public data
			std::vector<std::unique_ptr<Entry>>		m_entries;
		};

	}

}