#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "MapGenerator.h"
#include "Parser.h"

namespace tpublic
{

	class Manifest;
	class MapData;

	class MapGeneratorBase
	{
	public:
		MapGeneratorBase(
			MapGenerator::Id									aId)
			: m_id(aId)
		{

		}

		virtual 
		~MapGeneratorBase()
		{

		}

		bool
		FromSourceBase(
			const SourceNode*									/*aSource*/)
		{
			return false;
		}

		void	
		ToStreamBase(
			IWriter*											/*aStream*/) const 
		{
		}
		
		bool	
		FromStreamBase(
			IReader*											/*aStream*/) 
		{
			return true;
		}

		// Virtual methods
		virtual void			FromSource(
									const SourceNode*			/*aSource*/) { assert(false); }
		virtual void			ToStream(
									IWriter*					/*aStream*/) const { assert(false); }
		virtual bool			FromStream(
									IReader*					/*aStream*/) { assert(false); return true; }		
		virtual bool			Build(
									const Manifest*				/*aManifest*/,
									uint32_t					/*aSeed*/,
									const MapData*				/*aSourceMapData*/,
									std::unique_ptr<MapData>&	/*aOutMapData*/) const { assert(false); return false; }

		// Public data
		MapGenerator::Id		m_id;
	};

}