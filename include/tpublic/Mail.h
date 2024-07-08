#pragma once

#include "IReader.h"
#include "ItemInstance.h"
#include "IWriter.h"

namespace tpublic
{

	class Manifest;

	class Mail
	{
	public:
		static const uint32_t FORMAT_VERSION = 2;

		static const size_t MAX_ATTACHED_ITEMS = 8;

		enum Flag : uint8_t
		{
			FLAG_AUTO_DELETE = 0x01
		};

		void		ToStream(
						IWriter*		aWriter) const;
		bool		FromStream(
						IReader*		aReader);
		void		SetAutoDelete();
		bool		ShouldAutoDelete() const;
		void		FromString(
						const char*		aString,
						const Manifest*	aManifest,
						uint32_t		aOverrideToCharacterId = 0);

		// Public data
		uint32_t							m_toCharacterId = 0;
		uint32_t							m_fromCharacterId = 0;
		std::string							m_fromName;
		std::vector<ItemInstance>			m_items;
		int64_t								m_cash = 0;
		std::string							m_subject;
		std::string							m_body;
		uint8_t								m_flags = 0;
	};

}