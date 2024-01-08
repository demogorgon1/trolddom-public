#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct TagContext
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_TAG_CONTEXT;
			static const bool TAGGED = false;

			struct Scoring
			{							
				Scoring()
				{

				}

				Scoring(
					const SourceNode*		aNode)
				{
					m_tagId = aNode->m_sourceContext->m_persistentIdTable->GetId(DataType::ID_TAG, aNode->m_name.c_str());
					m_score = aNode->GetUInt32();
				}

				void
				ToStream(
					IWriter*				aStream) const 
				{
					aStream->WriteUInt(m_tagId);
					aStream->WriteUInt(m_score);
				}

				bool
				FromStream(
					IReader*				aStream) 
				{
					if (!aStream->ReadUInt(m_tagId))
						return false;
					if (!aStream->ReadUInt(m_score))
						return false;
					return true;
				}

				// Public data
				uint32_t			m_tagId = 0;
				uint32_t			m_score = 0;
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
						if (aChild->m_name == "must_have_tags")
							aChild->GetIdArray(DataType::ID_TAG, m_mustHaveTags);
						else if (aChild->m_name == "must_not_have_tags")
							aChild->GetIdArray(DataType::ID_TAG, m_mustHaveTags);
						else if (aChild->m_tag == "tag_score")
							m_scoring.push_back(Scoring(aChild));
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
				
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);

				aStream->WriteUInts(m_mustHaveTags);
				aStream->WriteUInts(m_mustNotHaveTags);
				aStream->WriteObjects(m_scoring);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;

				if(!aStream->ReadUInts(m_mustHaveTags))
					return false;
				if (!aStream->ReadUInts(m_mustNotHaveTags))
					return false;
				if(!aStream->ReadObjects(m_scoring))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>		m_mustHaveTags;
			std::vector<uint32_t>		m_mustNotHaveTags;
			std::vector<Scoring>		m_scoring;
		};

	}

}