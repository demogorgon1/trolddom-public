#include "Pcheader.h"

#include "Tokenizer.h"

namespace tpublic
{

	namespace
	{
		
		bool	
		_IsAlpha(
			char		aCharacter)
		{
			return (aCharacter >= 'a' && aCharacter <= 'z') || (aCharacter >= 'A' && aCharacter <= 'Z');
		}

		bool
		_IsNum(
			char		aCharacter)
		{
			return aCharacter >= '0' && aCharacter <= '9';
		}

		bool
		_IsWhitespace(
			char		aCharacter)
		{
			switch(aCharacter)
			{
			case ' ':
			case '\t':
			case '\0':
			case '\r':
			case '\n':
				return true;
			default:
				break;
			}
			return false;
		}

	}

	//-----------------------------------------------------------------------------------------------------

	Tokenizer::Tokenizer(
		const char*		aPath)
		: m_i(0)
	{
		// Determine base path
		{
			std::filesystem::path p = aPath;
			p.make_preferred();
			m_path = p.parent_path().string();
		}

		// Load and tokenize file
		{
			FILE* f = fopen(aPath, "rb");
			TP_CHECK(f != NULL, "Failed to open: %s", aPath);

			fseek(f, 0, SEEK_END);
			size_t fileSize = (size_t)ftell(f);
			fseek(f, 0, SEEK_SET);

			std::vector<char> buffer;
			buffer.resize(fileSize + 1, 0);
			fread(&buffer[0], 1, fileSize, f);

			fclose(f);

			_Tokenize(aPath, &buffer[0]);
		}
	}
	
	Tokenizer::~Tokenizer()
	{

	}

	bool			
	Tokenizer::IsEnd() const
	{
		const Token& token = Next();
		return token.m_type == Token::TYPE_END;
	}
	
	const Tokenizer::Token& 
	Tokenizer::Next() const
	{
		assert(m_i < m_tokens.size());
		return m_tokens[m_i];
	}
	
	void			
	Tokenizer::Proceed()
	{
		assert(m_i < m_tokens.size());
		m_i++;
	}

	const std::string& 
	Tokenizer::ConsumeAnyIdentifier()
	{
		const Token& token = Next();
		TP_VERIFY(token.m_type == Token::TYPE_IDENTIFIER, token.m_debugInfo, "Unexpected '%s', expected identifier.", token.m_value.c_str());
		Proceed();
		return token.m_value;
	}

	bool				
	Tokenizer::IsIdentifier()
	{
		const Token& token = Next();
		return token.m_type == Token::TYPE_IDENTIFIER;
	}

	bool				
	Tokenizer::IsToken(
		const char*		aToken)
	{
		const Token& token = Next();
		return token.m_value == aToken;
	}

	void				
	Tokenizer::ConsumeToken(
		const char*		aToken)
	{
		const Token& token = Next();
		TP_VERIFY(token.m_value == aToken, token.m_debugInfo, "Unexpected '%s', expected '%s'.", token.m_value.c_str(), aToken);
		Proceed();
	}

	//-----------------------------------------------------------------------------------------------------

	void	
	Tokenizer::_Tokenize(
		const char*		aPath,
		const char*		aString)
	{
		enum State
		{
			STATE_INIT,
			STATE_IDENTIFIER,
			STATE_NUMBER,
			STATE_STRING,
			STATE_STRING_ESCAPE,
			STATE_COMMENT
		};

		State state = STATE_INIT;
		const char* p = aString;
		std::vector<char> buffer;

		DataErrorHandling::DebugInfo debugInfo = { aPath, 1 };

		// Convert to unix style path
		{
			size_t pathLen = debugInfo.m_file.size();
			for(size_t iPath = 0; iPath < pathLen; iPath++)
			{
				if(debugInfo.m_file[iPath] == '\\')
					debugInfo.m_file[iPath] = '/';
			}
		}

		std::optional<char> next; 

		for(;;)
		{
			if(!next.has_value())
			{
				next = *p;
				p++;
			}

			char c = next.value();
			next.reset();

			if(c == '\n')
				debugInfo.m_line++;

			switch(state)
			{
			case STATE_INIT:
				if(c == '_' || _IsAlpha(c))
				{
					buffer.clear();
					buffer.push_back(c);
					state = STATE_IDENTIFIER;
				}
				else if(_IsNum(c) || c == '-' || c == '+')
				{
					buffer.clear();

					if(c != '+')
						buffer.push_back(c);

					state = STATE_NUMBER;
				}
				else if (c == '\"')
				{
					buffer.clear();
					state = STATE_STRING;
				}
				else if(c == '#')
				{
					state = STATE_COMMENT;
				}
				else if(!_IsWhitespace(c))
				{
					buffer.clear();
					buffer.push_back(c);
					buffer.push_back('\0');
					m_tokens.push_back({ Token::TYPE_CHARACTER, &buffer[0], debugInfo });
				}
				break;

			case STATE_IDENTIFIER:
				if(c == '_' || _IsAlpha(c) || _IsNum(c))
				{
					buffer.push_back(c);
				}
				else
				{
					buffer.push_back('\0');
					m_tokens.push_back({ Token::TYPE_IDENTIFIER, &buffer[0], debugInfo });

					next = c;
					state = STATE_INIT;
				}
				break;

			case STATE_NUMBER:
				if(c == '.' || _IsAlpha(c) || _IsNum(c))
				{
					buffer.push_back(c);
				}
				else
				{
					buffer.push_back('\0');
					m_tokens.push_back({ Token::TYPE_NUMBER, &buffer[0], debugInfo });

					next = c;
					state = STATE_INIT;
				}
				break;

			case STATE_STRING:
				if(c == '\"')
				{
					buffer.push_back('\0');
					m_tokens.push_back({ Token::TYPE_STRING, &buffer[0], debugInfo });
					state = STATE_INIT;
				}
				else if(c == '\\')
				{
					state = STATE_STRING_ESCAPE;
				}
				else
				{
					TP_VERIFY(c >= ' ', debugInfo, "Invalid string.");
					buffer.push_back(c);
				}
				break;

			case STATE_STRING_ESCAPE:
				if(c == 'n')
				{
					buffer.push_back('\n');
					state = STATE_STRING;
				}
				else if (c == '\"')
				{
					buffer.push_back('\"');
					state = STATE_STRING;
				}
				else
				{
					TP_VERIFY(false, debugInfo, "Invalid string escape code: '%c'", c);
				}
				break;

			case STATE_COMMENT:
				if(c == '\n')
					state = STATE_INIT;
				break;
			}

			if (c == '\0')
				break;
		}

		m_tokens.push_back({ Token::TYPE_END, "", debugInfo });
	}

}