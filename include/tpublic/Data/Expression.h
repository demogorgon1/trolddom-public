#pragma once

#include "../DataBase.h"
#include "../Expr.h"

namespace tpublic
{

	namespace Data
	{

		struct Expression
			: public DataBase
		{
			static const DataType::Id DATA_TYPE = DataType::ID_EXPRESSION;
			static const bool TAGGED = false;

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
				m_root = std::make_unique<Expr::Node>(aSource, Expr::OP_AND);
			}

			void
			ToStream(
				IWriter*				aStream) const override
			{
				aStream->WriteOptionalObjectPointer(m_root);
			}

			bool
			FromStream(
				IReader*				aStream) override
			{
				if(!aStream->ReadOptionalObjectPointer(m_root))
					return false;
				return true;
			}

			// Public data
			std::unique_ptr<Expr::Node>		m_root;
		};

	}

}