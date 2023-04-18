#include "Pcheader.h"

#include <kpublic/ItemInstanceData.h>
#include <kpublic/Manifest.h>

namespace kpublic
{

	ItemInstanceData::ItemInstanceData()
	{
	
	}

	ItemInstanceData::ItemInstanceData(
		std::mt19937&		aRandom,
		const Manifest*		aManifest,
		const ItemInstance&	aItemInstance)
		: m_instance(aItemInstance)
	{
		m_itemData = aManifest->m_items.GetById(aItemInstance.m_itemId);

		if(m_itemData->m_root)
			_Generate(aRandom, aManifest, m_itemData->m_root.get());
	}
	
	ItemInstanceData::~ItemInstanceData()
	{

	}

	//---------------------------------------------------------

	void		
	ItemInstanceData::_Generate(
		std::mt19937&			/*aRandom*/,
		const Manifest*			/*aManifest*/,
		const Data::Item::Node*	/*aNode*/)
	{
		
	}

}