#pragma once

#include "Hash.h"
#include "ImageResampler.h"

namespace tpublic
{

	class ImageResamplerCache
	{
	public:
		struct Key
		{
			struct Hasher
			{
				uint32_t
				operator()(
					const Key&				aKey) const
				{
					return (uint32_t)Hash::Combine_4_32(aKey.m_sourceWidth, aKey.m_sourceHeight, aKey.m_targetWidth, aKey.m_targetHeight);
				}
			};

			bool
			operator ==(
				const Key&					aOther) const
			{
				return m_sourceWidth == aOther.m_sourceWidth && m_sourceHeight == aOther.m_sourceHeight && m_targetWidth == aOther.m_targetWidth && m_targetHeight == aOther.m_targetHeight;
			}

			// Public data
			uint32_t	m_sourceWidth = 0;
			uint32_t	m_sourceHeight = 0;
			uint32_t	m_targetWidth = 0;
			uint32_t	m_targetHeight = 0;
		};

		const ImageResampler*		Get(
										const Key&							aKey) const;
		void						Insert(
										const Key&							aKey,
										std::unique_ptr<ImageResampler>&	aImageResampler);

	private:

		typedef std::unordered_map<Key, std::unique_ptr<ImageResampler>, Key::Hasher> Table;
		Table			m_table;
	};

}