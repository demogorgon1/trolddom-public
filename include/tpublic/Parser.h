#pragma once

#include "DataType.h"
#include "DataErrorHandling.h"
#include "Macro.h"
#include "PersistentIdTable.h"
#include "SourceContext.h"
#include "SourceNode.h"

namespace tpublic
{

	class Tokenizer;

	class Parser
	{
	public:
							Parser(
								SourceContext*		aSourceContext);
							~Parser();

		void				Parse(
								Tokenizer&			aTokenizer);
		void				ResolveMacrosAndReferences();
		void				ResolveEmbeddedDataObjects();
		void				DebugPrint() const;

		// Data access
		const SourceNode*	GetRoot() const { return &m_root; }

	private:

		struct MacroNamespace
		{
			void
			InsertMacro(
				std::unique_ptr<Macro>&						aMacro)
			{
				TP_VERIFY(!m_table.contains(aMacro->m_name), aMacro->m_debugInfo, "'%s' already exists in macro namespace.", aMacro->m_name.c_str());
				m_table[aMacro->m_name] = std::move(aMacro);
			}

			const Macro*
			GetMacro(
				const char*									aName) const
			{
				Table::const_iterator i = m_table.find(aName);
				if(i != m_table.cend())
					return i->second.get();
				return NULL;
			}

			// Public data
			typedef std::unordered_map<std::string, std::unique_ptr<Macro>> Table;
			Table			m_table;
		};

		SourceNode									m_root;

		typedef std::unordered_map<std::string, std::unique_ptr<MacroNamespace>> MacroNamespaceTable;
		MacroNamespaceTable							m_macroNamespaceTable;

		std::vector<std::unique_ptr<SourceNode>>	m_referenceObjects;
		
		void					_ParseObject(
									SourceNode*						aNamespace,
									Tokenizer&						aTokenizer,
									const char*						aEndToken,
									SourceNode*						aObject);
		void					_ParseArray(
									SourceNode*						aNamespace,
									Tokenizer&						aTokenizer,
									SourceNode*						aArray);
		void					_ParseValue(
									SourceNode*						aNamespace,
									Tokenizer&						aTokenizer,
									SourceNode*						aParent);
		void					_ParseExpression(
									SourceNode*						aNamespace,
									Tokenizer&						aTokenizer,
									SourceNode*						aParent);
		void					_DebugPrint(
									const SourceNode*				aNode,
									uint32_t						aLevel) const;
		MacroNamespace*			_GetOrCreateMacroNamespace(
									const char*						aPath);
		const MacroNamespace*	_GetMacroNamespace(
									const char*						aPath) const;
		void					_ResolveMacrosAndReferences(
									SourceNode*						aNode);
		void					_InnerResolveMacrosAndReferences(
									SourceNode*						aNode);
		void					_ResolveEmbeddedDataObjects(
									std::vector<std::string>&		aObjectNameStack,
									SourceNode*						aNamespace,
									SourceNode*						aNode);
		const Macro*			_FindMacro(
									const char*						aPath,
									const char*						aName);
		SourceNode*				_ExprOrs(
									Tokenizer&						aTokenizer);
		SourceNode*				_ExprAnds(
									Tokenizer&						aTokenizer);
		SourceNode*				_ExprEquals(
									Tokenizer&						aTokenizer);
		SourceNode*				_ExprSummands(
									Tokenizer&						aTokenizer);
		SourceNode*				_ExprFactors(
									Tokenizer&						aTokenizer);
		SourceNode*				_ExprExponentials(
									Tokenizer&						aTokenizer);
		SourceNode*				_ExprOperand(
									Tokenizer&						aTokenizer);
		std::string				_EvaluateExpression(
									const SourceNode*				aSourceNode);

	};
	
}