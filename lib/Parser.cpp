#include "Pcheader.h"

#include <tpublic/Parser.h>
#include <tpublic/Tokenizer.h>

namespace tpublic
{

	namespace
	{

		double
		_StringToBool(
			const std::string&						aValue)
		{
			if (aValue == "true")
				return 1.0;
			if (aValue == "false")
				return 0.0;
			if (aValue == "yes")
				return 1.0;
			if (aValue == "no")
				return 0.0;
			
			return std::stod(aValue) != 0.0;
		}

		const SourceNode* 
		_FindReferenceObject(
			const std::vector<std::unique_ptr<SourceNode>>&	aReferenceObjects,
			const char*								aName)
		{
			if (aReferenceObjects.size() == 0)
				return NULL;

			const SourceNode* resolved = NULL;

			for (int32_t j = (int32_t)aReferenceObjects.size() - 1; j >= 0 && resolved == NULL; j--)
			{
				aReferenceObjects[j]->GetObject()->ForEachChild([&](
					const SourceNode* aMember)
				{
					if (aMember->m_name == aName)
						resolved = aMember;
				});
			}

			return resolved;
		}

		void
		_TrimNumber(
			std::string&							aValue)
		{
			// Turn "3.4124000" into "3.4124" and "1.000" into "1"
			std::optional<size_t> offset;
			size_t length = aValue.length();

			for(size_t i = 0; i < length; i++)
			{
				char c = aValue[i];

				if(c == '.')
				{
					if(offset.has_value())
						return;

					offset = i;
				}
				else if(!(c >= '0' && c <= '9'))
				{
					return;
				}
			}

			if(offset.has_value())
			{
				// Floating point number, remove tailing zeroes
				assert(offset.value() < length);

				while(length > 0)
				{
					if(aValue[length - 1] == '0')
					{
						aValue.resize(length - 1);
						length--;
					}
					else
					{
						break;
					}
				}

				assert(length > 0);

				if(aValue[length - 1] == '.')
					aValue.resize(length - 1);
			}
		}

	}

	//------------------------------------------------------------------------------

	Parser::Parser(
		SourceContext*	aSourceContext)
		: m_root(aSourceContext, DataErrorHandling::DebugInfo("", 0), "", "", "")
	{

	}
	
	Parser::~Parser()
	{

	}

	void
	Parser::Parse(
		Tokenizer&	aTokenizer)
	{
		_ParseObject(aTokenizer, NULL, &m_root);
	}

	void				
	Parser::ResolveMacrosAndReferences()
	{
		_ResolveMacrosAndReferences(&m_root);
	}

	void	
	Parser::DebugPrint() const
	{
		_DebugPrint(&m_root, 0);
	}

	//------------------------------------------------------------------------------

	void
	Parser::_ParseObject(
		Tokenizer&	aTokenizer,
		const char* aEndToken,
		SourceNode*	aObject)
	{
		aObject->m_type = SourceNode::TYPE_OBJECT;

		while(!aTokenizer.IsEnd())
		{
			if(aEndToken != NULL && aTokenizer.IsToken(aEndToken))
			{
				aTokenizer.Proceed();
				break;
			}

			if (aTokenizer.IsToken("$"))
			{
				aTokenizer.Proceed();
				aTokenizer.ConsumeToken("{");

				std::unique_ptr<SourceNode> node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				_ParseObject(aTokenizer, "}", node.get());
				
				node->m_type = SourceNode::TYPE_REFERENCE_OBJECT;

				aObject->m_children.push_back(std::move(node));
			}
			else if(aTokenizer.IsToken("!"))
			{
				aTokenizer.Proceed();
				aTokenizer.ConsumeToken("define");

				bool isLocal = aTokenizer.TryConsumeToken("local");

				// Macro definition
				const std::string& identifier = aTokenizer.ConsumeAnyIdentifier();
					
				std::unique_ptr<SourceNode> body = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				_ParseValue(aTokenizer, body.get());

				std::unique_ptr<Macro> macro = std::make_unique<Macro>(identifier.c_str(), body->m_debugInfo);
				macro->m_body = std::move(body);

				// Insert macro in namespace
				_GetOrCreateMacroNamespace(isLocal ? aTokenizer.GetPathWithFileName() : aTokenizer.GetPath())->InsertMacro(macro);
			}
			else
			{
				std::unique_ptr<SourceNode> node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);

				if (aTokenizer.IsToken("<"))
				{
					aTokenizer.Proceed();
				
					std::unique_ptr<SourceNode> expression = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
					_ParseExpression(aTokenizer, expression.get());

					node->m_condition = std::move(expression);

					aTokenizer.ConsumeToken(">");
				}

				const std::string& identifier = aTokenizer.ConsumeAnyIdentifier();

				if (aTokenizer.IsIdentifier())
				{
					node->m_name = aTokenizer.ConsumeAnyIdentifier();
					node->m_tag = identifier;
				}
				else 
				{
					node->m_name = identifier;
				}

				if(aTokenizer.IsToken("<"))
				{
					aTokenizer.Proceed();

					std::unique_ptr<SourceNode> annotation = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
					_ParseValue(aTokenizer, annotation.get());

					node->m_annotation = std::move(annotation);

					aTokenizer.ConsumeToken(">");
				}

				aTokenizer.ConsumeToken(":");

				_ParseValue(aTokenizer, node.get());

				aObject->m_children.push_back(std::move(node));
			}
		}
	}

	void	
	Parser::_ParseArray(
		Tokenizer&		aTokenizer,
		SourceNode*		aArray)
	{
		aArray->m_type = SourceNode::TYPE_ARRAY;

		while (!aTokenizer.IsEnd())
		{
			if (aTokenizer.IsToken("]"))
			{
				aTokenizer.Proceed();
				break;
			}
			
			std::unique_ptr<SourceNode> node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);

			if (aTokenizer.IsToken("<"))
			{
				aTokenizer.Proceed();

				std::unique_ptr<SourceNode> expression = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				_ParseExpression(aTokenizer, expression.get());

				node->m_condition = std::move(expression);

				aTokenizer.ConsumeToken(">");
			}

			_ParseValue(aTokenizer, node.get());

			aArray->m_children.push_back(std::move(node));
		}
	}

	void	
	Parser::_ParseValue(
		Tokenizer&		aTokenizer,
		SourceNode*		aParent)
	{
		if(aTokenizer.IsToken("!"))
		{
			aTokenizer.Proceed();
			const std::string& macroName = aTokenizer.ConsumeAnyIdentifier();

			aParent->m_type = SourceNode::TYPE_MACRO_INVOCATION;

			// Macro name
			{
				std::unique_ptr<SourceNode> node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				node->m_type = SourceNode::TYPE_IDENTIFIER;
				node->m_value = macroName;
				aParent->m_children.push_back(std::move(node));
			}

			// Macro arguments
			{
				std::unique_ptr<SourceNode> node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);

				_ParseValue(aTokenizer, node.get());

				aParent->m_children.push_back(std::move(node));
			}
		}
		else if (aTokenizer.IsToken("@"))
		{
			aTokenizer.Proceed();
			const std::string& referenceName = aTokenizer.ConsumeAnyIdentifier();

			aParent->m_type = SourceNode::TYPE_REFERENCE;

			// Reference name
			{
				std::unique_ptr<SourceNode> node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				node->m_type = SourceNode::TYPE_IDENTIFIER;
				node->m_value = referenceName;
				aParent->m_children.push_back(std::move(node));
			}
		}
		else if (aTokenizer.IsToken("{"))
		{	
			aTokenizer.Proceed();
			_ParseObject(aTokenizer, "}", aParent);	
		}
		else if (aTokenizer.IsToken("["))
		{
			aTokenizer.Proceed();
			_ParseArray(aTokenizer, aParent);
		}
		else
		{
			const Tokenizer::Token& token = aTokenizer.Next();
			aTokenizer.Proceed();

			if (token.m_type == Tokenizer::Token::TYPE_NUMBER)
				aParent->m_type = SourceNode::TYPE_NUMBER;
			else if (token.m_type == Tokenizer::Token::TYPE_STRING)
				aParent->m_type = SourceNode::TYPE_STRING;
			else if (token.m_type == Tokenizer::Token::TYPE_IDENTIFIER)
				aParent->m_type = SourceNode::TYPE_IDENTIFIER;
			else
				TP_VERIFY(false, token.m_debugInfo, "Unexpected '%s', expected value.", token.m_value.c_str());

			aParent->m_value = token.m_value;
		}
	}

	void
	Parser::_ParseExpression(
		Tokenizer&			aTokenizer,
		SourceNode*			aParent)
	{
		aParent->m_type = SourceNode::TYPE_EXPRESSION;
		aParent->m_children.push_back(std::unique_ptr<SourceNode>(_ExprOrs(aTokenizer)));
	}

	void	
	Parser::_DebugPrint(
		const SourceNode*	aNode,
		uint32_t			aLevel) const
	{
		for(uint32_t i = 0; i < aLevel; i++)
			printf("  ");

		switch(aNode->m_type)
		{
		case SourceNode::TYPE_OBJECT:				printf("object "); break;
		case SourceNode::TYPE_ARRAY:				printf("array "); break;
		case SourceNode::TYPE_NUMBER:				printf("number "); break;
		case SourceNode::TYPE_STRING:				printf("string "); break;
		case SourceNode::TYPE_IDENTIFIER:			printf("identifier "); break;
		case SourceNode::TYPE_MACRO_INVOCATION:		printf("macro_invocation "); break;
		case SourceNode::TYPE_REFERENCE:			printf("reference "); break;
		case SourceNode::TYPE_EXPRESSION:			printf("expression "); break;
		case SourceNode::TYPE_EXPRESSION_OR:		printf("expression_or "); break;
		case SourceNode::TYPE_EXPRESSION_AND:		printf("expression_and "); break;
		case SourceNode::TYPE_EXPRESSION_EQUAL:		printf("expression_equal "); break;
		case SourceNode::TYPE_EXPRESSION_ADD:		printf("expression_add "); break;
		case SourceNode::TYPE_EXPRESSION_SUBTRACT:	printf("expression_subtract "); break;
		case SourceNode::TYPE_EXPRESSION_MULTIPLY:	printf("expression_multiply "); break;
		case SourceNode::TYPE_EXPRESSION_DIVIDE:	printf("expression_divide "); break;
		case SourceNode::TYPE_EXPRESSION_POWER:		printf("expression_power "); break;
		case SourceNode::TYPE_EXPRESSION_EXISTS:	printf("expression_exists "); break;
		case SourceNode::TYPE_EXPRESSION_NEGATE:	printf("expression_negate "); break;
		case SourceNode::TYPE_EXPRESSION_NOT:		printf("expression_not "); break;
		case SourceNode::TYPE_EXPRESSION_VALUE:		printf("expression_value "); break;
		default:									assert(false);
		}

		printf("[%s][%s][%s]\n", aNode->m_tag.c_str(), aNode->m_name.c_str(), aNode->m_value.c_str());
		
		if(aNode->m_annotation)
		{
			for (uint32_t i = 0; i < aLevel; i++)
				printf("  ");
			printf("annotation:\n");
			_DebugPrint(aNode->m_annotation.get(), aLevel + 1);
		}

		if(aNode->m_condition)
		{
			for (uint32_t i = 0; i < aLevel; i++)
				printf("  ");
			printf("condition:\n");
			_DebugPrint(aNode->m_condition.get(), aLevel + 1);
		}

		for(const std::unique_ptr<SourceNode>& child : aNode->m_children)
			_DebugPrint(child.get(), aLevel + 1);
	}

	Parser::MacroNamespace*
	Parser::_GetOrCreateMacroNamespace(
		const char*			aPath)
	{
		MacroNamespaceTable::iterator i = m_macroNamespaceTable.find(aPath);
		if(i != m_macroNamespaceTable.end())
			return i->second.get();

		MacroNamespace* t = new MacroNamespace();
		m_macroNamespaceTable[aPath] = std::unique_ptr<MacroNamespace>(t);
		return t;
	}

	const Parser::MacroNamespace* 
	Parser::_GetMacroNamespace(
		const char*			aPath) const
	{
		MacroNamespaceTable::const_iterator i = m_macroNamespaceTable.find(aPath);
		if (i != m_macroNamespaceTable.cend())
			return i->second.get();
		return NULL;
	}

	void				
	Parser::_ResolveMacrosAndReferences(
		SourceNode*						aNode)
	{
		for(size_t i = 0; i < aNode->m_children.size(); i++)
		{
			std::unique_ptr<SourceNode>& child = aNode->m_children[i];
			
			if(child->m_condition)
			{
				assert(child->m_condition->m_children.size() == 1);
				std::string result = _EvaluateExpression(child->m_condition->m_children[0].get());
				bool condition = std::stod(result) != 0.0f;
				
				if(!condition)
				{
					aNode->m_children.erase(aNode->m_children.begin() + i);
					i--;
					continue;
				}

				child->m_condition.reset();
			}

			if(child->m_type == SourceNode::TYPE_REFERENCE_OBJECT)
			{
				child->m_type = SourceNode::TYPE_OBJECT;
				m_referenceObjects.push_back(std::move(child));

				aNode->m_children.erase(aNode->m_children.begin() + i);
				i--;
				continue;
			}

			if (child->m_annotation)
				_InnerResolveMacrosAndReferences(child->m_annotation.get());

			_InnerResolveMacrosAndReferences(child.get());
		}
	}

	void
	Parser::_InnerResolveMacrosAndReferences(
		SourceNode*						aNode)
	{
		size_t originalReferenceObjectCount = m_referenceObjects.size();

		switch (aNode->m_type)
		{
		case SourceNode::TYPE_MACRO_INVOCATION:
			{
				assert(aNode->m_children.size() == 2);
				const char* macroName = aNode->m_children[0]->GetIdentifier();
				std::unique_ptr<SourceNode> arg = std::move(aNode->m_children[1]);

				const Macro* macro = _FindMacro(aNode->m_pathWithFileName.c_str(), macroName);
				TP_VERIFY(macro != NULL, aNode->m_debugInfo, "'%s' is not a defined macro.", macroName);

				aNode->Copy(macro->m_body.get());

				if (arg->m_type == SourceNode::TYPE_OBJECT)
					m_referenceObjects.push_back(std::move(arg));
			}
			break;

		case SourceNode::TYPE_REFERENCE:
			{
				assert(aNode->m_children.size() == 1);
				const char* referenceName = aNode->m_children[0]->GetIdentifier();
				const SourceNode* resolved = _FindReferenceObject(m_referenceObjects, referenceName);
				TP_VERIFY(resolved != NULL, aNode->m_debugInfo, "'%s' is not defined.", referenceName);

				std::unique_ptr<SourceNode> annotation = std::move(aNode->m_annotation);

				aNode->Copy(resolved);

				aNode->m_annotation = std::move(annotation);
			}
			break;

		default:
			break;
		}

		assert(aNode->m_type != SourceNode::TYPE_REFERENCE && aNode->m_type != SourceNode::TYPE_MACRO_INVOCATION);

		_ResolveMacrosAndReferences(aNode);

		// Pop off any reference objects
		if (originalReferenceObjectCount != m_referenceObjects.size())
		{
			assert(originalReferenceObjectCount < m_referenceObjects.size());
			m_referenceObjects.resize(originalReferenceObjectCount);
		}
	}

	const Macro* 
	Parser::_FindMacro(
		const char*			aPath,
		const char*			aName)
	{
		std::filesystem::path path = aPath;
		
		for(;;)
		{
			const MacroNamespace* macroNamespace = _GetMacroNamespace(path.string().c_str());
			if(macroNamespace != NULL)
			{
				const Macro* macro = macroNamespace->GetMacro(aName);
				if(macro != NULL)
					return macro;
			}

			if(path.empty())
				break;

			path = path.parent_path();
		} 

		return NULL;
	}

	SourceNode*
	Parser::_ExprOrs(
		Tokenizer&			aTokenizer)
	{
		std::unique_ptr<SourceNode> lhs(_ExprAnds(aTokenizer));
		while (!aTokenizer.IsToken(">"))
		{
			if (aTokenizer.IsToken("or"))
			{
				aTokenizer.Proceed();
				std::unique_ptr<SourceNode> rhs(_ExprAnds(aTokenizer));
				std::unique_ptr<SourceNode> newLhs = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				newLhs->m_type = SourceNode::TYPE_EXPRESSION_OR;
				newLhs->m_children.push_back(std::move(lhs));
				newLhs->m_children.push_back(std::move(rhs));
				lhs = std::move(newLhs);
				continue;
			}
			return lhs.release();
		}
		return lhs.release();
	}

	SourceNode*
	Parser::_ExprAnds(
		Tokenizer&			aTokenizer)
	{
		std::unique_ptr<SourceNode> lhs(_ExprEquals(aTokenizer));
		while (!aTokenizer.IsToken(">"))
		{
			if (aTokenizer.IsToken("and"))
			{
				aTokenizer.Proceed();
				std::unique_ptr<SourceNode> rhs(_ExprEquals(aTokenizer));
				std::unique_ptr<SourceNode> newLhs = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				newLhs->m_type = SourceNode::TYPE_EXPRESSION_AND;
				newLhs->m_children.push_back(std::move(lhs));
				newLhs->m_children.push_back(std::move(rhs));
				lhs = std::move(newLhs);
				continue;
			}
			return lhs.release();
		}
		return lhs.release();
	}

	SourceNode*
	Parser::_ExprEquals(
		Tokenizer&			aTokenizer)
	{
		std::unique_ptr<SourceNode> lhs(_ExprSummands(aTokenizer));
		while (!aTokenizer.IsToken(">"))
		{
			if (aTokenizer.IsToken("equals"))
			{
				aTokenizer.Proceed();
				std::unique_ptr<SourceNode> rhs(_ExprSummands(aTokenizer));
				std::unique_ptr<SourceNode> newLhs = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				newLhs->m_type = SourceNode::TYPE_EXPRESSION_EQUAL;
				newLhs->m_children.push_back(std::move(lhs));
				newLhs->m_children.push_back(std::move(rhs));
				lhs = std::move(newLhs);
				continue;
			}
			return lhs.release();
		}
		return lhs.release();
	}

	SourceNode*
	Parser::_ExprSummands(
		Tokenizer&			aTokenizer)
	{
		std::unique_ptr<SourceNode> lhs(_ExprFactors(aTokenizer));
		while (!aTokenizer.IsToken(">"))
		{
			if (aTokenizer.IsToken("+"))
			{
				aTokenizer.Proceed();
				std::unique_ptr<SourceNode> rhs(_ExprFactors(aTokenizer));
				std::unique_ptr<SourceNode> newLhs = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				newLhs->m_type = SourceNode::TYPE_EXPRESSION_ADD;
				newLhs->m_children.push_back(std::move(lhs));
				newLhs->m_children.push_back(std::move(rhs));
				lhs = std::move(newLhs);
				continue;
			}
			else if (aTokenizer.IsToken("-"))
			{
				aTokenizer.Proceed();
				std::unique_ptr<SourceNode> rhs(_ExprFactors(aTokenizer));
				std::unique_ptr<SourceNode> newLhs = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				newLhs->m_type = SourceNode::TYPE_EXPRESSION_SUBTRACT;
				newLhs->m_children.push_back(std::move(lhs));
				newLhs->m_children.push_back(std::move(rhs));
				lhs = std::move(newLhs);
				continue;
			}
			return lhs.release();
		}
		return lhs.release();
	}

	SourceNode*
	Parser::_ExprFactors(
		Tokenizer&			aTokenizer)
	{
		std::unique_ptr<SourceNode> lhs(_ExprExponentials(aTokenizer));
		while (!aTokenizer.IsToken(">"))
		{
			if (aTokenizer.IsToken("*"))
			{
				aTokenizer.Proceed();
				std::unique_ptr<SourceNode> rhs(_ExprExponentials(aTokenizer));
				std::unique_ptr<SourceNode> newLhs = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				newLhs->m_type = SourceNode::TYPE_EXPRESSION_MULTIPLY;
				newLhs->m_children.push_back(std::move(lhs));
				newLhs->m_children.push_back(std::move(rhs));
				lhs = std::move(newLhs);
				continue;
			}
			else if (aTokenizer.IsToken("/"))
			{
				aTokenizer.Proceed();
				std::unique_ptr<SourceNode> rhs(_ExprExponentials(aTokenizer));
				std::unique_ptr<SourceNode> newLhs = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				newLhs->m_type = SourceNode::TYPE_EXPRESSION_DIVIDE;
				newLhs->m_children.push_back(std::move(lhs));
				newLhs->m_children.push_back(std::move(rhs));
				lhs = std::move(newLhs);
				continue;
			}
			return lhs.release();
		}
		return lhs.release();
	}

	SourceNode*
	Parser::_ExprExponentials(
		Tokenizer&			aTokenizer)
	{
		std::unique_ptr<SourceNode> lhs(_ExprOperand(aTokenizer));
		while (!aTokenizer.IsToken(">"))
		{
			if (aTokenizer.IsToken("^"))
			{
				aTokenizer.Proceed();
				std::unique_ptr<SourceNode> rhs(_ExprOperand(aTokenizer));
				std::unique_ptr<SourceNode> newLhs = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				newLhs->m_type = SourceNode::TYPE_EXPRESSION_POWER;
				newLhs->m_children.push_back(std::move(lhs));
				newLhs->m_children.push_back(std::move(rhs));
				lhs = std::move(newLhs);
				continue;
			}
			return lhs.release();
		}
		return lhs.release();
	}

	SourceNode*
	Parser::_ExprOperand(
		Tokenizer&			aTokenizer)
	{
		std::unique_ptr<SourceNode> node;

		if(aTokenizer.IsToken("("))
		{
			aTokenizer.Proceed();
			node.reset(_ExprOrs(aTokenizer));
		}
		else if (aTokenizer.IsToken("-"))
		{
			aTokenizer.Proceed();
			node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
			node->m_type = SourceNode::TYPE_EXPRESSION_NEGATE;
			node->m_children.push_back(std::unique_ptr<SourceNode>(_ExprOperand(aTokenizer)));
		}
		else if (aTokenizer.IsToken("!"))
		{
			aTokenizer.Proceed();
			node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
			node->m_type = SourceNode::TYPE_EXPRESSION_NOT;
			node->m_children.push_back(std::unique_ptr<SourceNode>(_ExprOperand(aTokenizer)));
		}
		else if (aTokenizer.IsIdentifier())
		{
			const std::string& identifier = aTokenizer.ConsumeAnyIdentifier();

			if(identifier == "exists")
			{
				node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				node->m_type = SourceNode::TYPE_EXPRESSION_EXISTS;
				node->m_name = aTokenizer.ConsumeAnyIdentifier();		
			}
			else
			{
				node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
				node->m_type = SourceNode::TYPE_EXPRESSION_VALUE;
				node->m_name = identifier;
			}
		}
		else 
		{
			TP_VERIFY(false, aTokenizer.Next().m_debugInfo, "Unexpected token in expression.");
		}

		assert(node);
		return node.release();
	}

	std::string				
	Parser::_EvaluateExpression(
		const SourceNode*	aSourceNode)
	{
		std::string result;

		try
		{
			switch(aSourceNode->m_type)
			{
			case SourceNode::TYPE_EXPRESSION_OR:
				{
					assert(aSourceNode->m_children.size() == 2);
					std::string lhs = _EvaluateExpression(aSourceNode->m_children[0].get());
					if(_StringToBool(lhs))
					{
						result = "1";
					}
					else
					{
						std::string rhs = _EvaluateExpression(aSourceNode->m_children[1].get());
						if (_StringToBool(rhs))
							result = "1";
						else
							result = "0";
					}
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_AND:
				{
					assert(aSourceNode->m_children.size() == 2);
					std::string lhs = _EvaluateExpression(aSourceNode->m_children[0].get());
					if(!_StringToBool(lhs))
					{
						result = "0";
					}
					else
					{
						std::string rhs = _EvaluateExpression(aSourceNode->m_children[1].get());
						if (!_StringToBool(rhs))
							result = "0";
						else
							result = "1";
					}
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_EQUAL:
				{
					assert(aSourceNode->m_children.size() == 2);
					std::string lhs = _EvaluateExpression(aSourceNode->m_children[0].get());
					std::string rhs = _EvaluateExpression(aSourceNode->m_children[1].get());
					result = std::to_string(lhs == rhs);
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_ADD:
				{
					assert(aSourceNode->m_children.size() == 2);
					std::string lhs = _EvaluateExpression(aSourceNode->m_children[0].get());
					std::string rhs = _EvaluateExpression(aSourceNode->m_children[1].get());
					result = std::to_string(std::stod(lhs) + std::stod(rhs));
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_SUBTRACT:
				{
					assert(aSourceNode->m_children.size() == 2);
					std::string lhs = _EvaluateExpression(aSourceNode->m_children[0].get());
					std::string rhs = _EvaluateExpression(aSourceNode->m_children[1].get());
					result = std::to_string(std::stod(lhs) - std::stod(rhs));
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_MULTIPLY:
				{
					assert(aSourceNode->m_children.size() == 2);
					std::string lhs = _EvaluateExpression(aSourceNode->m_children[0].get());
					std::string rhs = _EvaluateExpression(aSourceNode->m_children[1].get());
					result = std::to_string(std::stod(lhs) * std::stod(rhs));
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_DIVIDE:
				{
					assert(aSourceNode->m_children.size() == 2);
					std::string lhs = _EvaluateExpression(aSourceNode->m_children[0].get());
					std::string rhs = _EvaluateExpression(aSourceNode->m_children[1].get());
					result = std::to_string(std::stod(lhs) / std::stod(rhs));
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_POWER:
				{
					assert(aSourceNode->m_children.size() == 2);
					std::string lhs = _EvaluateExpression(aSourceNode->m_children[0].get());
					std::string rhs = _EvaluateExpression(aSourceNode->m_children[1].get());
					result = std::to_string(std::pow(std::stod(lhs), std::stod(rhs)));
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_EXISTS:
				{
					assert(aSourceNode->m_children.size() == 0);
					result = _FindReferenceObject(m_referenceObjects, aSourceNode->m_name.c_str()) != NULL ? "1" : "0";
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_VALUE:
				{
					assert(aSourceNode->m_children.size() == 0);
					const SourceNode* referenceObject = _FindReferenceObject(m_referenceObjects, aSourceNode->m_name.c_str());
					TP_VERIFY(referenceObject != NULL, aSourceNode->m_debugInfo, "'%s' is not defined.", aSourceNode->m_name.c_str());
					result = referenceObject->m_value;
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_NEGATE:
				{
					assert(aSourceNode->m_children.size() == 1);
					std::string value = _EvaluateExpression(aSourceNode->m_children[0].get());
					result = std::to_string(std::stod(value) == 0.0 ? 1.0 : 0.0);
				}			
				break;

			case SourceNode::TYPE_EXPRESSION_NOT:
				{
					assert(aSourceNode->m_children.size() == 1);
					std::string value = _EvaluateExpression(aSourceNode->m_children[0].get());
					result = std::to_string(std::stod(value) == 0.0 ? 1.0 : 0.0);
				}			
				break;

			default:
				assert(false);
				break;
			}
		}
		catch(...)
		{
			TP_VERIFY(false, aSourceNode->m_debugInfo, "Invalid expression.");
		}

		_TrimNumber(result);

		return result;
	}

}