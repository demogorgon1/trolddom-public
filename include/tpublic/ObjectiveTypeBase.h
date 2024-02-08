#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "ObjectiveType.h"

namespace tpublic
{

	class ObjectiveInstanceBase;
	class SourceNode;

	class ObjectiveTypeBase
	{
	public:
		ObjectiveTypeBase(
			ObjectiveType::Id										aObjectiveTypeId)
			: m_objectiveTypeId(aObjectiveTypeId)
		{

		}

		virtual 
		~ObjectiveTypeBase()
		{

		}

		// Virtual interface
		virtual void					FromSource(
											const SourceNode*		aSource) = 0;
		virtual void					ToStream(
											IWriter*				aWriter) = 0;
		virtual bool					FromStream(
											IReader*				aReader) = 0;
		virtual ObjectiveInstanceBase*	CreateInstance() const = 0;

		// Virtual methods
		virtual void					GetWatchedEntities(
											std::vector<uint32_t>&	/*aOutEntityIds*/) const { }

		// Data access
		ObjectiveType::Id				GetObjectiveTypeId() const { return m_objectiveTypeId; }

	private:
		ObjectiveType::Id		m_objectiveTypeId;
	};

}