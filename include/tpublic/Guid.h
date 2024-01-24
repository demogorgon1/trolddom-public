#pragma once

namespace tpublic
{

	class Guid
	{
	public:
		static const size_t SIZE = 16;

		static Guid			Null();		

							Guid();
							Guid(
								const Guid&		aCopy);
							Guid(
								const char*		aString);
							~Guid();
		Guid&				operator=(
								const Guid&		aCopy);
		bool				IsSet() const;
		const uint8_t*		GetBuffer() const;
		uint8_t*			GetBuffer();	
		void				ToString(
								std::string&	aOut) const;
		std::string			AsString() const;
		uint32_t			GetHash32() const;

	private:

		uint8_t			m_data[SIZE];
	};

	inline bool operator == (const Guid& a1, const Guid& a2) { return memcmp(a1.GetBuffer(), a2.GetBuffer(), Guid::SIZE) == 0; }
	inline bool operator != (const Guid& a1, const Guid& a2) { return memcmp(a1.GetBuffer(), a2.GetBuffer(), Guid::SIZE) != 0; }

}
