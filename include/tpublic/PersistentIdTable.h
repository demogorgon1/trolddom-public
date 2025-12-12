#pragma once

#include "DataErrorHandling.h"
#include "DataType.h"

namespace tpublic
{

	class PersistentIdTable
	{
	public:
		typedef std::function<void(const char*, const char*, const DataErrorHandling::DebugInfo&)> UndefinedCallback;

						PersistentIdTable();
						~PersistentIdTable();

		uint32_t		GetId(
							const DataErrorHandling::DebugInfo&								aDebugInfo,
							DataType::Id													aDataType,
							const char*														aName,
							bool															aIsDefinition = false);
		void			Load(
							const char*														aPath);
		void			Save();
		void			ValidateAndPrune(
							bool															aValidateOnly,
							UndefinedCallback												aUndefinedCallback);

	private:

		struct ReferenceDebugInfo
		{
			DataErrorHandling::DebugInfo									m_debugInfo;
			bool															m_isDefinition = false;
		};

		struct Type
		{
			std::unordered_map<std::string, uint32_t>						m_table;
			uint32_t														m_nextId = 1;

			// Build-time debug information, not serialized
			std::unordered_map<uint32_t, std::vector<ReferenceDebugInfo>>	m_debugInfoReferences;
		};

		std::string															m_path;
		Type																m_types[DataType::NUM_IDS];
	};

}