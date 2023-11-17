#pragma once

#include "DataErrorHandling.h"

namespace tpublic
{

	class Tokenizer
	{
	public:
		struct Token
		{
			enum Type
			{
				TYPE_END,
				TYPE_IDENTIFIER,
				TYPE_NUMBER,
				TYPE_STRING,
				TYPE_CHARACTER
			};
		
			// Public data
			Type							m_type;
			std::string						m_value;
			DataErrorHandling::DebugInfo	m_debugInfo;
		};

							Tokenizer(
								const char*				aPath);
							~Tokenizer();

		bool				IsEnd() const;
		const Token&		Next() const;
		void				Proceed();
		const std::string&	ConsumeAnyIdentifier();
		bool				IsIdentifier();
		bool				IsToken(
								const char*				aToken);
		void				ConsumeToken(
								const char*				aToken);
		bool				TryConsumeToken(
								const char*				aToken);

		// Data access
		const char*			GetPath() const { return m_path.c_str(); }
		const char*			GetPathWithFileName() const { return m_pathWithFileName.c_str(); }

	private:

		std::vector<Token>				m_tokens;
		size_t							m_i;
		std::string						m_path;
		std::string						m_pathWithFileName;

		void	_Tokenize(
					const char*							aPath,
					const char*							aString);
	};

}