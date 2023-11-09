#pragma once

#include "../DataBase.h"

namespace tpublic
{

	namespace Data
	{

		struct LootGroup
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_LOOT_GROUP;

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const SourceNode*		/*aNode*/) override
			{
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				ToStreamBase(aStream);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if (!FromStreamBase(aStream))
					return false;
				return true;
			}

			// Public data
		};

	}

}