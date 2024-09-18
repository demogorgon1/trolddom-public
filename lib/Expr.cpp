#include "Pcheader.h"

#include <tpublic/Expr.h>

namespace tpublic
{

	namespace Expr
	{

		void			
		Context::InitDefault()
		{
			m_opFunctions[OP_AND] = [](
				const Args* aArgs) -> int64_t
			{
				for(size_t i = 0; i < aArgs->m_count; i++)
				{
					if (!aArgs->m_values[i])
						return 0;
				}
				return 1;
			};

			m_opFunctions[OP_OR] = [](
				const Args* aArgs) -> int64_t
			{
				for(size_t i = 0; i < aArgs->m_count; i++)
				{
					if (aArgs->m_values[i])
						return 1;
				}
				return 0;
			};

			m_opFunctions[OP_NOT] = [](
				const Args* aArgs) -> int64_t
			{
				TP_CHECK(aArgs->m_count == 1, "Invalid operation.");
				return aArgs->m_values[0] == 0 ? 1 : 0;
			};

			m_opFunctions[OP_EQUAL] = [](
				const Args* aArgs) -> int64_t
			{
				TP_CHECK(aArgs->m_count == 2, "Invalid operation.");
				return aArgs->m_values[0] == aArgs->m_values[1] ? 1 : 0;
			};

			m_opFunctions[OP_NOT_EQUAL] = [](
				const Args* aArgs) -> int64_t
			{
				TP_CHECK(aArgs->m_count == 2, "Invalid operation.");
				return aArgs->m_values[0] != aArgs->m_values[1] ? 1 : 0;
			};

			m_opFunctions[OP_LESS_THAN_OR_EQUAL] = [](
				const Args* aArgs) -> int64_t
			{
				TP_CHECK(aArgs->m_count == 2, "Invalid operation.");
				return aArgs->m_values[0] <= aArgs->m_values[1] ? 1 : 0;
			};

			m_opFunctions[OP_GREATER_THAN_OR_EQUAL] = [](
				const Args* aArgs) -> int64_t
			{
				TP_CHECK(aArgs->m_count == 2, "Invalid operation.");
				return aArgs->m_values[0] >= aArgs->m_values[1] ? 1 : 0;
			};

			m_opFunctions[OP_LESS_THAN] = [](
				const Args* aArgs) -> int64_t
			{
				TP_CHECK(aArgs->m_count == 2, "Invalid operation.");
				return aArgs->m_values[0] < aArgs->m_values[1] ? 1 : 0;
			};

			m_opFunctions[OP_GREATER_THAN] = [](
				const Args* aArgs) -> int64_t
			{
				TP_CHECK(aArgs->m_count == 2, "Invalid operation.");
				return aArgs->m_values[0] > aArgs->m_values[1] ? 1 : 0;
			};
		}

		void			
		Context::SetParent(
			const Context*	aParent)
		{
			m_parent = aParent;
		}
		
		void			
		Context::SetOpFunction(
			Op				aOp,
			OpFunction		aOpFunction)
		{
			m_opFunctions[aOp] = aOpFunction;
		}

		int64_t
		Context::ExecuteNode(
			const Node*		aNode) const
		{			
			if(aNode->m_constant.has_value())
				return aNode->m_constant.value();

			Args args;
			args.m_count = aNode->m_children.size();
			TP_CHECK(args.m_count <= Args::MAX_COUNT, "Too many expression node children.");

			for(size_t i = 0; i < aNode->m_children.size(); i++)
				args.m_values[i] = ExecuteNode(aNode->m_children[i].get());

			return ExecuteOp(aNode->m_op, &args);
		}

		int64_t
		Context::ExecuteOp(
			Op				aOp,
			const Args*		aArgs) const
		{
			if(aOp == INVALID_OP)
				return 0;

			const OpFunction& opFunction = m_opFunctions[aOp];
			
			if(opFunction)
				return opFunction(aArgs);

			TP_CHECK(m_parent != NULL, "Operation not implemented in context: %u", aOp);
			return m_parent->ExecuteOp(aOp, aArgs);
		}

	}

}