#include "Pcheader.h"

#include <tpublic/Data/Doodad.h>
#include <tpublic/Data/Sprite.h>

#include <tpublic/Manifest.h>

namespace tpublic
{

	namespace PostProcessDoodads
	{

		void		
		Run(
			Manifest* aManifest)
		{
			aManifest->GetContainer<Data::Doodad>()->ForEach([aManifest](
				Data::Doodad* aDoodad)
			{
				if(aDoodad->m_spriteIds.size() > 0)
				{
					const Data::Sprite* sprite = aManifest->GetById<Data::Sprite>(aDoodad->m_spriteIds[0]);

					aDoodad->m_sizeInTiles.m_x = (int32_t)sprite->m_width / 16;
					aDoodad->m_sizeInTiles.m_y = (int32_t)sprite->m_height / 16;
				}
				return true;
			});
		}

	}

}