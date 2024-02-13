#pragma once

namespace tpublic
{

	class IReader;
	class IWriter;

	class SoundData
	{
	public:
		struct Entry
		{
			void		ToStream(
							IWriter*				aWriter) const;
			bool		FromStream(
							IReader*				aReader);

			// Public data
			std::vector<uint8_t>		m_data;
		};

		void			ToStream(
							IWriter*				aWriter) const;
		bool			FromStream(
							IReader*				aReader);
		void			AddEntry(
							uint32_t				aSoundId,
							std::vector<uint8_t>&	aData);
		const Entry*	GetEntry(
							uint32_t				aSoundId) const;

	private:
		
		typedef std::unordered_map<uint32_t, std::unique_ptr<Entry>> Table;
		Table							m_table;
	};

}