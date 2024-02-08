#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "Parser.h"

namespace tpublic
{

	namespace Expr
	{

		enum Op : uint8_t
		{
			INVALID_OP,

			// Logical
			OP_AND,
			OP_OR,
			OP_NOT,
			OP_EQUAL,
			OP_NOT_EQUAL,
			OP_LESS_THAN_OR_EQUAL,
			OP_GREATER_THAN_OR_EQUAL,
			OP_LESS_THAN,
			OP_GREATER_THAN,

			// Client 
			OP_CLIENT_PLAYER_IS_IN_GUILD,
			OP_CLIENT_WALLET_CASH,
			OP_CLIENT_GUILD_REGISTRATION_PRICE,

			NUM_OPS
		};

		// IMPORTANT: must match Op enum
		static constexpr const char* OPS[] =
		{
			NULL,

			// Logical
			"and",
			"or",
			"not",
			"equal",
			"not_equal",
			"less_than_or_equal",
			"greater_than_or_equal",
			"less_than",
			"greater_than",

			// Client
			"client_player_is_in_guild",
			"client_wallet_cash",
			"client_guild_registration_price"
		};

		static_assert(sizeof(OPS) / sizeof(const char*) == (size_t)NUM_OPS);

		inline constexpr Op
		StringToOp(
			const char* aString)
		{
			std::string_view s(aString);
			for (uint8_t i = 1; i < (uint8_t)NUM_OPS; i++)
			{
				if (s == OPS[i])
					return (Op)i;
			}

			return INVALID_OP;
		}

		struct Node
		{
			Node()
			{

			}

			Node(
				const SourceNode*	aSource,
				Op					aOp)
				: m_op(aOp)
			{
				TP_VERIFY(m_op != INVALID_OP, aSource->m_debugInfo, "'%s' is not a valid operation.", aSource->m_name.c_str());

				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					m_children.push_back(std::make_unique<Node>(aChild, StringToOp(aChild->m_name.c_str())));
				});
			}

			void
			ToStream(
				IWriter*			aWriter) const
			{
				aWriter->WritePOD(m_op);
				aWriter->WriteObjectPointers(m_children);
			}

			bool
			FromStream(
				IReader*			aReader)
			{
				if(!aReader->ReadPOD(m_op))
					return false;
				if(!aReader->ReadObjectPointers(m_children))
					return false;
				return true;
			}			

			// Public data
			Op										m_op = INVALID_OP;
			std::vector<std::unique_ptr<Node>>		m_children;
		};

		struct Args
		{
			static const size_t MAX_COUNT = 8;

			// Public data
			size_t		m_count = 0; 
			uint32_t	m_values[MAX_COUNT] = { 0 };	
		};

		typedef std::function<uint32_t(const Args*)> OpFunction;

		class Context
		{
		public:
			void			InitDefault();
			void			SetParent(
								const Context*	aParent);
			void			SetOpFunction(
								Op				aOp,
								OpFunction		aOpFunction);
			uint32_t		ExecuteNode(
								const Node*		aNode) const;
			uint32_t		ExecuteOp(
								Op				aOp,
								const Args*		aArgs) const;
		
		private:

			OpFunction								m_opFunctions[NUM_OPS];
			const Context*							m_parent = NULL;
		};
	}

}