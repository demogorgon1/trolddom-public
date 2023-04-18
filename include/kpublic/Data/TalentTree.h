#pragma once

#include "../DataBase.h"

namespace kpublic
{

	namespace Data
	{

		struct TalentTree
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_TALENT_TREE;

			void
			Verify() const
			{
				VerifyBase();
			}

			// Base implementation
			void
			FromSource(
				const Parser::Node*		/*aNode*/) override
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