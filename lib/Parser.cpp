#include "Pcheader.h"

#include <tpublic/Helpers.h>
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

		const SourceNode*
		_GetStackSourceNodeByName(
			const std::vector<const SourceNode*>&	aStack,
			const char*								aName)
		{
			for(size_t i = 0; i < aStack.size(); i++)
			{
				const SourceNode* child = aStack[aStack.size() - i - 1]->TryGetChildByName(aName);
				if(child != NULL)
					return child;
			}
			return NULL;
		}

		bool
		_IsInsideObject(
			const SourceNode*						aRoot,
			const std::vector<const SourceNode*>&	aStack)
		{
			for(const SourceNode* t : aStack)
			{
				if(t != aRoot && !t->IsAnonymousObject())
					return true;;
			}
			return false;
		}

		const char*
		_ResolveTag(
			const std::vector<const SourceNode*>&	aStack,
			const char*								aTag)
		{
			const char* tagName = NULL;

			for (size_t i = 0; i < aStack.size() && tagName == NULL; i++)
			{
				const SourceNode* t = aStack[aStack.size() - i - 1];
				if (t->m_tag == aTag)
					tagName = t->m_name.c_str();
			}

			return tagName;
		}

	}

	//------------------------------------------------------------------------------

	Parser::Parser(
		SourceContext*	aSourceContext)
		: m_root(aSourceContext, DataErrorHandling::DebugInfo{"", 0}, "", "", "")
	{

	}
	
	Parser::~Parser()
	{

	}

	void
	Parser::Parse(
		Tokenizer&		aTokenizer)
	{
		_ParseObject(&m_root, aTokenizer, NULL, &m_root);
	}

	void				
	Parser::Resolve()
	{	
		// Substitute/insert macros and resolve references
		{
			_ResolveMacrosAndReferences(&m_root);
		}

		// Resolve <name-of-item-on-stack> identifiers 
		{
			std::vector<const SourceNode*> stack;

			_ResolveContextTags(stack, &m_root);
		}

		// Resolve inline/embedded data objects
		{
			std::vector<std::string> objectNameStack;
			std::vector<const SourceNode*> stack;

			_ResolveEmbeddedDataObjects(stack, objectNameStack, &m_root, &m_root);
		}
	}

	void	
	Parser::DebugPrint() const
	{
		_DebugPrint(&m_root, 0);
	}

	//------------------------------------------------------------------------------

	void
	Parser::_ParseObject(
		SourceNode*		aNamespace,
		Tokenizer&		aTokenizer,
		const char*		aEndToken,
		SourceNode*		aObject)
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
				_ParseObject(aNamespace, aTokenizer, "}", node.get());
				
				node->m_type = SourceNode::TYPE_REFERENCE_OBJECT;

				aObject->m_children.push_back(std::move(node));
			}
			else if(aTokenizer.IsToken("!"))
			{
				aTokenizer.Proceed();

				if(aTokenizer.IsToken("define"))
				{
					aTokenizer.Proceed();

					bool isLocal = aTokenizer.TryConsumeToken("local");

					// Macro definition
					const std::string& identifier = aTokenizer.ConsumeAnyIdentifier();

					std::unique_ptr<SourceNode> body = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
					_ParseValue(aNamespace, aTokenizer, body.get());

					std::unique_ptr<Macro> macro = std::make_unique<Macro>(identifier.c_str(), body->m_debugInfo);
					macro->m_body = std::move(body);

					// Insert macro in namespace
					_GetOrCreateMacroNamespace(isLocal ? aTokenizer.GetPathWithFileName() : aTokenizer.GetPath())->InsertMacro(macro);
				}
				else 
				{
					std::unique_ptr<SourceNode> node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
					node->m_type = SourceNode::TYPE_MACRO_OBJECT_IMPORT;
					node->m_name = aTokenizer.ConsumeAnyIdentifier();
					aObject->m_children.push_back(std::move(node));

					if(aTokenizer.IsToken("{"))
					{
						aTokenizer.Proceed();
						TP_VERIFY(aTokenizer.IsToken("}"), node->m_debugInfo, "Arguments not allowed for macro object imports.");
						aTokenizer.Proceed();
					}
				}
			}
			else
			{
				std::unique_ptr<SourceNode> node = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);

				if (aTokenizer.IsToken("<"))
				{
					aTokenizer.Proceed();
				
					std::unique_ptr<SourceNode> expression = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
					_ParseExpression(aNamespace, aTokenizer, expression.get());

					node->m_condition = std::move(expression);

					aTokenizer.ConsumeToken(">");
				}

				if(!aTokenizer.IsToken("{"))
				{
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
						_ParseValue(aNamespace, aTokenizer, annotation.get());

						node->m_annotation = std::move(annotation);

						aTokenizer.ConsumeToken(">");
					}

					if (aTokenizer.IsToken("("))
					{
						aTokenizer.Proceed();

						std::unique_ptr<SourceNode> annotation = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
						_ParseValue(aNamespace, aTokenizer, annotation.get());

						node->m_extraAnnotation = std::move(annotation);

						aTokenizer.ConsumeToken(")");
					}

					aTokenizer.ConsumeToken(":");

					if(aTokenizer.IsToken("$"))
					{
						aTokenizer.Proceed();

						std::unique_ptr<SourceNode> embeddedObject = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);

						embeddedObject->m_tag = aTokenizer.ConsumeAnyIdentifier();

						if (aTokenizer.IsToken("<"))
						{
							aTokenizer.Proceed();

							std::unique_ptr<SourceNode> annotation = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
							_ParseValue(aNamespace, aTokenizer, annotation.get());

							embeddedObject->m_annotation = std::move(annotation);

							aTokenizer.ConsumeToken(">");
						}

						if (aTokenizer.IsToken("("))
						{
							aTokenizer.Proceed();

							std::unique_ptr<SourceNode> annotation = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
							_ParseValue(aNamespace, aTokenizer, annotation.get());

							embeddedObject->m_extraAnnotation = std::move(annotation);

							aTokenizer.ConsumeToken(")");
						}

						std::string name;

						if (aTokenizer.IsToken("@"))
						{
							aTokenizer.Proceed();

							if (aTokenizer.IsToken("<"))
							{
								aTokenizer.Proceed();

								if(aTokenizer.IsToken(">"))
								{
									name = "?";
								}
								else
								{
									name = "!"; // Indicate it's a tag that needs to be resolved
									name += aTokenizer.ConsumeAnyIdentifier();
								}

								aTokenizer.ConsumeToken(">");
							}
							else
							{
								name = aTokenizer.ConsumeAnyIdentifier();
							}
						}

						std::unique_ptr<SourceNode> importArray;

						if(aTokenizer.IsToken("["))
						{
							aTokenizer.Proceed();
							importArray = std::make_unique<SourceNode>(m_root.m_sourceContext, aTokenizer);
							_ParseArray(aNamespace, aTokenizer, importArray.get());
						}

						_ParseValue(aNamespace, aTokenizer, embeddedObject.get());

						embeddedObject->m_name = std::move(name);

						node->m_type = SourceNode::TYPE_EMBEDDED_DATA_OBJECT;
						node->m_children.push_back(std::move(embeddedObject));

						if(importArray)
							node->m_children.push_back(std::move(importArray));
					}
				}			

				if(node->m_type != SourceNode::TYPE_EMBEDDED_DATA_OBJECT)
					_ParseValue(aNamespace, aTokenizer, node.get());

				aObject->m_children.push_back(std::move(node));
			}
		}
	}

	void	
	Parser::_ParseArray(
		SourceNode*		aNamespace,
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
				_ParseExpression(aNamespace, aTokenizer, expression.get());

				node->m_condition = std::move(expression);

				aTokenizer.ConsumeToken(">");
			}

			_ParseValue(aNamespace, aTokenizer, node.get());

			aArray->m_children.push_back(std::move(node));
		}
	}

	void	
	Parser::_ParseValue(
		SourceNode*		aNamespace,
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

				_ParseValue(aNamespace, aTokenizer, node.get());

				aParent->m_children.push_back(std::move(node));
			}
		}
		else if(aTokenizer.IsToken("<"))
		{
			aTokenizer.Proceed();
			const std::string& identifier = aTokenizer.ConsumeAnyIdentifier();
			aTokenizer.ConsumeToken(">");

			aParent->m_type = SourceNode::TYPE_CONTEXT_TAG;
			aParent->m_value = identifier;
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

			// New namespace?
			SourceNode* nextNamespace = aNamespace;
			if((aParent->m_type == SourceNode::TYPE_NONE && aParent->m_name.empty()) || aNamespace == &m_root)
				nextNamespace = aParent;

			_ParseObject(nextNamespace, aTokenizer, "}", aParent);
		}
		else if (aTokenizer.IsToken("["))
		{
			aTokenizer.Proceed();
			_ParseArray(aNamespace, aTokenizer, aParent);
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
		SourceNode*			/*aNamespace*/,
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
		case SourceNode::TYPE_EMBEDDED_DATA_OBJECT:	printf("embedded_data_object "); break;
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

			if(child->m_type == SourceNode::TYPE_MACRO_OBJECT_IMPORT)
			{
				const Macro* macro = _FindMacro(child->m_path.c_str(), child->m_name.c_str());
				TP_VERIFY(macro != NULL, child->m_debugInfo, "'%s' is not a valid macro.", child->m_name.c_str());
				DataErrorHandling::DebugInfo debugInfo = child->m_debugInfo;
				aNode->m_children.erase(aNode->m_children.begin() + i);
				
				size_t j = i;
				for(const std::unique_ptr<SourceNode>& t : macro->m_body->m_children)
				{
					std::unique_ptr<SourceNode> node = std::make_unique<SourceNode>(m_root.m_sourceContext, debugInfo, t->m_realPath.c_str(), t->m_path.c_str(), t->m_pathWithFileName.c_str());
					node->m_tag = t->m_tag;
					node->m_name = t->m_name;
					node->Copy(t.get());
					aNode->m_children.insert(aNode->m_children.begin() + j, std::move(node));
					j++;
				}

				i--;
				continue;
			}

			if (child->m_annotation)
				_InnerResolveMacrosAndReferences(child->m_annotation.get());

			bool isMacroInvocation = child->m_type == SourceNode::TYPE_MACRO_INVOCATION;

			_InnerResolveMacrosAndReferences(child.get());		

			// If this is a macro invocation that produces a (non-empty) array, and we're currently in an array, do an append
			if(isMacroInvocation && child->m_type == SourceNode::TYPE_ARRAY && aNode->m_type == SourceNode::TYPE_ARRAY && child->m_children.size() > 0)
			{
				size_t j = i;

				std::unique_ptr<SourceNode> detached = std::move(child);

				for(std::unique_ptr<SourceNode>& t : detached->m_children)
				{					
					if(j == i)
						child = std::move(t);
					else
						aNode->m_children.insert(aNode->m_children.begin() + j, std::move(t));
					j++;
				}

				i = j + 1;
			}
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

	void					
	Parser::_ResolveEmbeddedDataObjects(
		std::vector<const SourceNode*>& aSourceNodeStack,
		std::vector<std::string>&		aObjectNameStack,
		SourceNode*						aNamespace,
		SourceNode*						aNode)
	{
		aSourceNodeStack.push_back(aNode);

		bool needObjectNameStackPop = false;

		std::string t;

		if (!aNode->m_tag.empty())
		{
			t += "_";
			t += aNode->m_tag;
		}

		if(!aNode->m_name.empty())
		{
			t += "_";
			t += aNode->m_name;
		}

		if(!t.empty())
		{
			aObjectNameStack.push_back(t);
			needObjectNameStackPop = true;
		}

		uint32_t multiSeqNum = 0;

		for (size_t i = 0; i < aNode->m_children.size(); i++)
		{
			std::unique_ptr<SourceNode>& child = aNode->m_children[i];

			if(child->m_type == SourceNode::TYPE_EMBEDDED_DATA_OBJECT)
			{
				assert(child->m_children.size() == 1 || child->m_children.size() == 2);

				std::unique_ptr<SourceNode> dataObject = std::move(child->m_children[0]);

				if(child->m_children.size() == 2)
				{
					assert(child->m_children[1]->m_type == SourceNode::TYPE_ARRAY);
					for(const std::unique_ptr<SourceNode>& importIdentifier : child->m_children[1]->m_children)
					{
						const char* identifier = importIdentifier->GetIdentifier();
						const SourceNode* toCopy = _GetStackSourceNodeByName(aSourceNodeStack, identifier);
						TP_VERIFY(toCopy != NULL, importIdentifier->m_debugInfo, "'%s' is not a valid parent child.", identifier);

						std::unique_ptr<SourceNode> copyObject = std::make_unique<SourceNode>(m_root.m_sourceContext, toCopy->m_debugInfo, toCopy->m_realPath.c_str(), toCopy->m_path.c_str(), toCopy->m_pathWithFileName.c_str());
						copyObject->Copy(toCopy);
						copyObject->m_name = toCopy->m_name;

						dataObject->m_children.push_back(std::move(copyObject));
					}
				}						

				child->m_children.clear();

				if(dataObject->m_name.empty() || dataObject->m_name == "?")
				{
					std::string embeddedObjectName;
					for (const std::string& objectName : aObjectNameStack)
						embeddedObjectName += objectName;

					if (!child->m_tag.empty())
					{
						embeddedObjectName += "_";
						embeddedObjectName += child->m_tag;
					}

					if (!child->m_name.empty())
					{
						embeddedObjectName += "_";
						embeddedObjectName += child->m_name;
					}

					if(dataObject->m_name == "?")
						embeddedObjectName += Helpers::Format("_%u", multiSeqNum++);

					dataObject->m_name = std::move(embeddedObjectName);
				}
				else if(dataObject->m_name[0] == '!')
				{
					const char* tag = dataObject->m_name.c_str() + 1;
					const char* tagName = _ResolveTag(aSourceNodeStack, tag);
					TP_VERIFY(tagName != NULL, child->m_debugInfo, "Unable to resolve tag '%s'.", tag);
					dataObject->m_name = tagName;
				}

				child->m_value = dataObject->m_name;
				child->m_type = SourceNode::TYPE_IDENTIFIER;

				aNamespace->m_children.push_back(std::move(dataObject));
			}
		}

		bool isInsideObject = _IsInsideObject(&m_root, aSourceNodeStack);

		uint32_t x = 0;
		for (std::unique_ptr<SourceNode>& child : aNode->m_children)
		{
			SourceNode* nextNamespace = aNamespace; 
			
			if(child->IsAnonymousObject() && !isInsideObject)
				nextNamespace = child.get();
				
			_ResolveEmbeddedDataObjects(aSourceNodeStack, aObjectNameStack, nextNamespace, child.get());
			x++;
		}

		if(needObjectNameStackPop)
			aObjectNameStack.pop_back();

		aSourceNodeStack.pop_back();
	}

	void					
	Parser::_ResolveContextTags(
		std::vector<const SourceNode*>& aStack,
		SourceNode*						aNode)
	{
		aStack.push_back(aNode);

		for (std::unique_ptr<SourceNode>& child : aNode->m_children)
		{			
			if(child->m_type == SourceNode::TYPE_CONTEXT_TAG)
			{
				const char* tagName = _ResolveTag(aStack, child->m_value.c_str());
				TP_VERIFY(tagName != NULL, child->m_debugInfo, "Unable to resolve tag '%s'.", child->m_value.c_str());

				child->m_type = SourceNode::TYPE_IDENTIFIER;
				child->m_value = tagName;
			}

			_ResolveContextTags(aStack, child.get());
		}

		aStack.pop_back();
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