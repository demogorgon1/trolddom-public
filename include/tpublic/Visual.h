#pragma once

namespace tpublic
{
	
	class Visual
	{
	public:
		enum Type : uint8_t
		{
			INVALID_TYPE,

			TYPE_TILE_SHAKE
		};

		enum Flag : uint8_t
		{
			FLAG_LINE_OF_SIGHT = 0x01
		};

		static Type
		SourceToType(
			const SourceNode*		aSource)
		{
			std::string_view t(aSource->GetIdentifier());
			if(t == "tile_shake")
				return TYPE_TILE_SHAKE;
			TP_VERIFY(false, aSource->m_debugInfo, "'%s' is not a valid visual type.", aSource->GetIdentifier());
			return INVALID_TYPE;
		}

		static uint8_t
		SourceToFlags(
			const SourceNode*		aSource)
		{
			uint8_t flags = 0;
			aSource->GetArray()->ForEachChild([&](
				const SourceNode* aChild)
			{
				std::string_view t(aChild->GetIdentifier());
				if(t == "line_of_sight")
					flags |= FLAG_LINE_OF_SIGHT;
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid flag.", aChild->GetIdentifier());
			});
			return flags;
		}

		Visual()
		{

		}

		Visual(
			const SourceNode*		aSource)
		{
			TP_VERIFY(aSource->m_annotation, aSource->m_debugInfo, "Missing type annotation.");
			m_type = SourceToType(aSource->m_annotation.get());

			aSource->GetObject()->ForEachChild([&](
				const SourceNode* aChild)
			{
				if(aChild->m_name == "radius")
					m_radius = aChild->GetInt32();
				else if(aChild->m_name == "flags")
					m_flags = SourceToFlags(aChild);
				else if(aChild->m_name == "duration")
					m_duration = aChild->GetInt32();
				else if (aChild->m_name == "amount")
					m_amount = aChild->GetInt32();
				else
					TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid item.", aChild->GetIdentifier());				
			});
		}

		void
		ToStream(
			IWriter*				aWriter) const
		{
			aWriter->WritePOD(m_type);
			aWriter->WritePOD(m_flags);
			aWriter->WriteInt(m_radius);
			aWriter->WriteInt(m_duration);
			aWriter->WriteInt(m_amount);
		}

		bool
		FromStream(
			IReader*				aReader) 
		{
			if (!aReader->ReadPOD(m_type))
				return false;
			if (!aReader->ReadPOD(m_flags))
				return false;
			if (!aReader->ReadInt(m_radius))
				return false;
			if (!aReader->ReadInt(m_duration))
				return false;
			if (!aReader->ReadInt(m_amount))
				return false;
			return true;
		}

		// Public data
		Type		m_type = INVALID_TYPE;
		uint8_t		m_flags = 0;
		int32_t		m_radius = 0;
		int32_t		m_duration = 0;
		int32_t		m_amount = 0;
	};

}