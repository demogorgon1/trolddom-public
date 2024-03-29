#pragma once

#include "Data/MapSegment.h"
#include "Data/MapSegmentConnector.h"

#include "Vec2.h"

namespace tpublic
{

	class Image;
	class Manifest;

	class MapSegmentInstance
	{
	public:
		struct Connector
		{			
			uint32_t							m_mapSegmentConnectorId = 0;
			std::vector<Vec2>					m_positions;
		};		

		void				Generate(						
								const Manifest*				aManifest,
								const Data::MapSegment*		aMapSegment,
								std::mt19937&				aRandom);		
		void				CreateDebugImage(
								const Manifest*				aManifest,
								Image&						aOut) const;
		const Connector*	GetConnector(		
								uint32_t					aMapSegmentConnectorId) const;

		// Public data
		const Data::MapSegment*					m_mapSegment = NULL;
		const uint32_t*							m_tileMap = NULL;
		const uint32_t*							m_coverTileMap = NULL;
		Vec2									m_size;

		std::vector<uint32_t>					m_generatedTileMap;
		std::vector<std::unique_ptr<Connector>>	m_connectors;
		std::vector<Data::MapSegment::Object>	m_objects;

	private:

		Connector*			_GetOrCreateConnector(
								uint32_t					aMapSegmentConnectorId);
		void				_GenerateRandomRoom(
								const Manifest*				aManifest,
								std::mt19937&				aRandom);
		uint32_t			_SampleGeneratedTileMap(
								const Vec2&					aPosition) const;
		bool				_HasConnectorAtPosition(
								const Vec2&					aPosition) const;
	};

}