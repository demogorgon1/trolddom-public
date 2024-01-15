#pragma once

#include "../DataBase.h"

#include "../ObjectiveTypeFactory.h"

namespace tpublic
{

	namespace Data
	{

		struct Objective
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_OBJECTIVE;
			static const bool TAGGED = false;

			void
			Verify() const
			{
				VerifyBase();
			}

			bool
			HasMap(
				uint32_t				aMapId) const
			{
				for(uint32_t t : m_mapIds)
				{
					if(t == aMapId)
						return true;
				}
				return false;
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		aSource) override
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(!FromSourceBase(aChild))
					{
						if (aChild->m_name == "string")
						{
							m_string = aChild->GetString();
						}
						else if (aChild->m_name == "maps")
						{
							aChild->GetIdArray(DataType::ID_MAP, m_mapIds);
						}
						else
						{
							TP_VERIFY(m_objectiveTypeId == ObjectiveType::INVALID_ID, aChild->m_debugInfo, "Objective type already defined.");
							m_objectiveTypeId = ObjectiveType::StringToId(aChild->m_name.c_str());
							TP_VERIFY(ObjectiveType::ValidateId(m_objectiveTypeId), aChild->m_debugInfo, "'%s' is not a valid objective type.", aChild->m_name.c_str());
							m_objectiveTypeBase.reset(aChild->m_sourceContext->m_objectiveTypeFactory->Create(m_objectiveTypeId));
							assert(m_objectiveTypeBase);
							m_objectiveTypeBase->FromSource(aChild);
						}
					}
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteString(m_string);
				aStream->WriteUInts(m_mapIds);
				aStream->WritePOD(m_objectiveTypeId);
				aStream->WriteObjectPointer(m_objectiveTypeBase);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadString(m_string))
					return false;
				if(!aStream->ReadUInts(m_mapIds))
					return false;
				if(!aStream->ReadPOD(m_objectiveTypeId))
					return false;
				if(!ObjectiveType::ValidateId(m_objectiveTypeId))
					return false;

				m_objectiveTypeBase.reset(aStream->GetObjectiveTypeFactory()->Create(m_objectiveTypeId));
				assert(m_objectiveTypeBase);
				if(!m_objectiveTypeBase->FromStream(aStream))
					return false;

				return true;
			}

			// Public data
			std::string							m_string;
			std::vector<uint32_t>				m_mapIds;
			ObjectiveType::Id					m_objectiveTypeId = ObjectiveType::INVALID_ID;
			std::unique_ptr<ObjectiveTypeBase>	m_objectiveTypeBase;
		};

	}

}