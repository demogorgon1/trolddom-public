#pragma once

namespace tpublic
{

	class IReader;
	class IWriter;
	class SourceNode;

	class Document
	{
	public:
		struct Node
		{
			enum Type : uint8_t
			{
				INVALID_TYPE,

				TYPE_COMPOUND,
				TYPE_HEADING,
				TYPE_TEXT,
				TYPE_LIST,
				TYPE_NEWLINE
			};

						Node() {}
						Node(
							const SourceNode*			aSource);
			void		ToStream(
							IWriter*					aWriter) const;
			bool		FromStream(
							IReader*					aReader);
			uint32_t	CalculateHash() const;		
			
			// Public data
			Type								m_type = INVALID_TYPE;
			std::vector<std::unique_ptr<Node>>	m_children;
			std::string							m_string;
		};

						Document() {}
						Document(
							const SourceNode*				aSource);

		void			ToStream(
							IWriter*						aWriter) const;
		bool			FromStream(
							IReader*						aReader);

		// Public data
		std::unique_ptr<Node>					m_root;
	};

}