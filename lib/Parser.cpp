#include "Pcheader.h"

#include <kaos-public/Parser.h>

#include "Tokenizer.h"

namespace kaos_public
{

	Parser::Parser(
		SourceContext*	aSourceContext)
		: m_root(aSourceContext, DataErrorHandling::DebugInfo("", 0), "")
	{

	}
	
	Parser::~Parser()
	{

	}

	void
	Parser::Parse(
		Tokenizer&	aTokenizer)
	{
		_ParseObject(aTokenizer, &m_root);
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
		Node*		aObject)
	{
		aObject->m_type = Node::TYPE_OBJECT;

		while(!aTokenizer.IsEnd())
		{
			if(aTokenizer.IsToken("}"))
			{
				aTokenizer.Proceed();
				break;
			}

			std::unique_ptr<Node> node = std::make_unique<Node>(m_root.m_sourceContext, aTokenizer.Next().m_debugInfo, aTokenizer.GetPath());

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

			KP_VERIFY(aTokenizer.IsToken(":"), aTokenizer.Next().m_debugInfo, "Unexpected '%s', expected ':'.", aTokenizer.Next().m_value.c_str());
			aTokenizer.Proceed();

			_ParseValue(aTokenizer, node.get());

			aObject->m_children.push_back(std::move(node));
		}
	}

	void	
	Parser::_ParseArray(
		Tokenizer&		aTokenizer,
		Node*			aArray)
	{
		aArray->m_type = Node::TYPE_ARRAY;

		while (!aTokenizer.IsEnd())
		{
			if (aTokenizer.IsToken("]"))
			{
				aTokenizer.Proceed();
				break;
			}
			
			std::unique_ptr<Node> node = std::make_unique<Node>(m_root.m_sourceContext, aTokenizer.Next().m_debugInfo, aTokenizer.GetPath());

			_ParseValue(aTokenizer, node.get());

			aArray->m_children.push_back(std::move(node));
		}
	}

	void	
	Parser::_ParseValue(
		Tokenizer&		aTokenizer,
		Node*			aParent)
	{
		if (aTokenizer.IsToken("{"))
		{	
			aTokenizer.Proceed();
			_ParseObject(aTokenizer, aParent);	
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
				aParent->m_type = Node::TYPE_NUMBER;
			else if (token.m_type == Tokenizer::Token::TYPE_STRING)
				aParent->m_type = Node::TYPE_STRING;
			else if (token.m_type == Tokenizer::Token::TYPE_IDENTIFIER)
				aParent->m_type = Node::TYPE_IDENTIFIER;
			else
				KP_VERIFY(false, token.m_debugInfo, "Unexpected '%s', expected value.", token.m_value.c_str());

			aParent->m_value = token.m_value;
		}
	}

	void	
	Parser::_DebugPrint(
		const Node*		aNode,
		uint32_t		aLevel) const
	{
		for(uint32_t i = 0; i < aLevel; i++)
			printf("  ");

		switch(aNode->m_type)
		{
		case Node::TYPE_OBJECT:		printf("object "); break;
		case Node::TYPE_ARRAY:		printf("array "); break;
		case Node::TYPE_NUMBER:		printf("number "); break;
		case Node::TYPE_STRING:		printf("string "); break;
		case Node::TYPE_IDENTIFIER:	printf("identifier "); break;
		default:					assert(false);
		}

		printf("[%s][%s][%s]\n", aNode->m_tag.c_str(), aNode->m_name.c_str(), aNode->m_value.c_str());

		for(const std::unique_ptr<Node>& child : aNode->m_children)
			_DebugPrint(child.get(), aLevel + 1);
	}

}