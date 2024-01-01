#pragma once

namespace tpublic::UTF8
{

	class Decoder
	{
	public:
					Decoder(
						const char*		aUTF8Text);

		void		SetEmitNullTermination(
						bool			aEmitNullTermination);
		bool		GetNextCharacterCode(
						uint32_t&		aOut,
						size_t*			aOutSize = NULL);
		bool		IsEnd() const;

	private:

		const uint8_t*			m_pc;
		uint32_t				m_state;
		bool					m_emitNullTermination;
		bool					m_ended;
	};

	class Encoder
	{
	public:
		void		EncodeCharacter(
						uint32_t		aCharacterCode);
		void		Clear();
		const char*	Finalize();
			
		// Member access
		const char*	GetBuffer() const { assert(m_finalized); return (const char*)&m_utf8[0]; }

	private:

		bool					m_finalized = false;
		std::vector<uint8_t>	m_utf8;
	};

}