#pragma once

#include "../Component.h"

namespace tpublic
{

	namespace Components
	{

		struct Sprite
			: public ComponentBase
		{
			static const Component::Id ID = Component::ID_SPRITE;
			static const uint8_t FLAGS = FLAG_REPLICATE_TO_OWNER | FLAG_REPLICATE_TO_OTHERS;
			static const Persistence::Id PERSISTENCE = Persistence::ID_NONE;

			Sprite()
				: ComponentBase(ID, FLAGS, PERSISTENCE)
			{

			}

			virtual 
			~Sprite()
			{

			}

			// ComponentBase implementation
			void
			FromSource(
				const Parser::Node*		aSource) override
			{
				aSource->ForEachChild([&](
					const Parser::Node*	aChild)
				{
					if (aChild->m_name == "frames")
						aChild->GetIdArray(DataType::ID_SPRITE, m_frames);
					else if(aChild->m_name == "frame_delay")
						m_frameDelay = aChild->GetUInt32();
					else if (aChild->m_name == "random_start_frame")
						m_randomStartFrame = aChild->GetBool();
					else
						TP_VERIFY(false, aChild->m_debugInfo, "'%s' is not a valid member.", aChild->m_name.c_str());
				});
			}

			void
			ToStream(
				IWriter* aStream) const override
			{
				aStream->WriteUInts(m_frames);
				aStream->WriteUInt(m_frameDelay);
				aStream->WritePOD(m_randomStartFrame);
			}

			bool
			FromStream(
				IReader* aStream) override
			{
				if(!aStream->ReadUInts(m_frames))
					return false;
				if(!aStream->ReadUInt(m_frameDelay))
					return false;
				if (!aStream->ReadUInt(m_randomStartFrame))
					return false;
				return true;
			}

			// Public data
			std::vector<uint32_t>		m_frames;
			uint32_t					m_frameDelay = 0;
			bool						m_randomStartFrame = false;
		};
	}

}