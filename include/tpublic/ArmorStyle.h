#pragma once

#include "Image.h"
#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	namespace ArmorStyle
	{

		// IMPORTANT: Never remove or reorder anything in this enum, only add new things
		enum Id : uint32_t
		{
			INVALID_ID,

			ID_NONE,
			ID_BROWN,
			ID_GREEN,
			ID_CYAN,
			ID_RED,
			ID_PURPLE,
			ID_BLACK,
			ID_WHITE,
			ID_YELLOW,

			NUM_IDS
		};

		struct Info
		{
			const char* m_name;
		};

		// IMPORTANT: Must match Id enum
		static constexpr const Info INFO[] =
		{
			{ NULL },

			{ "none" },
			{ "brown" },
			{ "green" },
			{ "cyan" },
			{ "red" },
			{ "purple" },
			{ "black" },
			{ "white" },
			{ "yellow" }
		};

		static_assert(sizeof(INFO) / sizeof(Info) == NUM_IDS);

		inline constexpr const Info*
		GetInfo(
			Id						aId)
		{
			assert((uint32_t)aId < (uint32_t)NUM_IDS);
			return &INFO[aId];
		}

		inline constexpr Id
		StringToId(
			const char*				aString)
		{
			std::string_view s(aString);
			for (uint32_t i = 1; i < (uint32_t)NUM_IDS; i++)
			{
				const Info* t = &INFO[i];
				if (s == t->m_name)
					return (Id)i;
			}
			return INVALID_ID;
		}

		inline constexpr bool
		ValidateId(
			Id						aId)
		{
			return (uint32_t)aId >= 1 && (uint32_t)aId < (uint32_t)NUM_IDS;
		}

		struct Visual
		{
			Visual()
			{

			}

			Visual(
				const SourceNode*	aSource)
			{
				aSource->ForEachChild([&](
					const SourceNode* aChild)
				{
					if(aChild->m_name == "armor_style")
					{
						m_id = StringToId(aChild->GetIdentifier());
						TP_VERIFY(m_id != INVALID_ID, aChild->m_debugInfo, "'%s' is not a valid armor style.", aChild->GetIdentifier());
					}
					else if(aChild->m_name == "teint_color")
					{
						m_teintColor = Image::RGBA(aChild);
					}
					else if(aChild->m_name == "decoration_color")
					{
						m_decorationColor = Image::RGBA(aChild);
					}
					else
					{
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->m_name.c_str());
					}
				});
			}

			void
			ToStream(
				IWriter*			aWriter) const
			{
				aWriter->WritePOD(m_id);
				aWriter->WritePOD(m_teintColor);
				aWriter->WritePOD(m_decorationColor);
			}

			bool
			FromStream(
				IReader*			aReader)
			{
				if (!aReader->ReadPOD(m_id))
					return false;
				if (!aReader->ReadPOD(m_teintColor))
					return false;
				if (!aReader->ReadPOD(m_decorationColor))
					return false;
				return true;
			}

			// Public data
			Id			m_id = ID_BROWN;
			Image::RGBA	m_teintColor = { 255, 255, 255, 255 };
			Image::RGBA	m_decorationColor = { 255, 255, 255, 0 };
		};

	}

}