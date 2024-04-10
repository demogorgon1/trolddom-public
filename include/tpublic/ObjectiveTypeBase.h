#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "ObjectiveType.h"

namespace tpublic
{

	namespace Components
	{
		struct Inventory;
	}

	class ObjectiveInstanceBase;
	class SourceNode;

	class ObjectiveTypeBase
	{
	public:
		enum Flag : uint32_t
		{
			FLAG_UNMANAGED = 0x00000001,
			FLAG_WATCH_INVENTORY = 0x00000002
		};

		ObjectiveTypeBase(
			ObjectiveType::Id										aObjectiveTypeId,
			uint32_t												aFlags)
			: m_objectiveTypeId(aObjectiveTypeId)
			, m_flags(aFlags)
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
		virtual void					PostCompletionInventoryUpdate(
											Components::Inventory*	/*aInventory*/) const { }

		// Data access
		ObjectiveType::Id				GetObjectiveTypeId() const { return m_objectiveTypeId; }
		uint32_t						GetFlags() const { return m_flags; }

	private:

		ObjectiveType::Id		m_objectiveTypeId;
		uint32_t				m_flags;
	};

}