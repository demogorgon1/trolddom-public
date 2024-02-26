#include "Pcheader.h"

#include <tpublic/Data/Sprite.h>

#include <tpublic/Manifest.h>

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
				aSprite->m_info.m_tileLayer = aManifest->m_tileLayering.GetTileLayer(aSprite);
				return true;
			});
		}

	}

}