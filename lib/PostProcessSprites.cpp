#include "Pcheader.h"

#include <tpublic/Data/Sprite.h>

#include <tpublic/Manifest.h>
#include <tpublic/TileLayering.h>

namespace tpublic
{

	namespace PostProcessSprites
	{

		void		
		Run(
			Manifest* aManifest)
		{
			aManifest->GetContainer<Data::Sprite>()->ForEach([aManifest](
				Data::Sprite* aSprite)
			{
				aSprite->m_info.m_tileLayer = aManifest->m_tileLayering->GetTileLayer(aSprite);

				if(aSprite->m_info.m_flags & SpriteInfo::FLAG_AUTOGLOW)
				{
					std::string glowSpriteName = aSprite->m_name + "_glow";
					aSprite->m_info.m_glowSpriteId = aManifest->GetExistingIdByName<Data::Sprite>(glowSpriteName.c_str());
				}
				return true;
			});
		}

	}

}