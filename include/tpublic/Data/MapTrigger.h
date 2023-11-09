#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct MapTrigger
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_MAP_TRIGGER;

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
						if (aChild->m_name == "persistent")
							m_persistent = aChild->GetBool();
						else if (aChild->m_name == "default")
							m_default = aChild->GetBool();
						else
							TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
				aStream->WritePOD(m_persistent);
				aStream->WritePOD(m_default);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				if (!aStream->ReadPOD(m_persistent))
					return false;
				if (!aStream->ReadPOD(m_default))
					return false;
				return true;
			}

			// Public data
			bool			m_persistent = false;
			bool			m_default = false;
		};

	}

}