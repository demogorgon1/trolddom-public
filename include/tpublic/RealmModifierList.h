#pragma once

#include "RealmModifier.h"

namespace tpublic
{

	class IReader;
	class IWriter;

	class RealmModifierList
	{
	public:
		static const uint32_t VERSION = 1;

		struct Entry
		{
			void		ToStream(
							IWriter*			aWriter) const;
			bool		FromStream(
							IReader*			aReader);

			// Public data
			RealmModifier::Id	m_id = RealmModifier::INVALID_ID;			
			float				m_float = 0.0f;
			bool				m_bool = false;
		};

		void			ToStream(
							IWriter*			aWriter) const;
		bool			FromStream(
							IReader*			aReader);
		void			SetDefined(
							bool				aDefined);
		void			Add(
							const Entry&		aEntry);
		const Entry*	Get(
							RealmModifier::Id	aId) const;
		bool			GetFlag(
							RealmModifier::Id	aId,
							bool				aDefault) const;
		float			GetMultiplier(
							RealmModifier::Id	aId,
							float				aDefault) const;
		
		// Data access
		bool			IsDefined() const { return m_defined; }

	private:

		uint32_t				m_version = 0;
		std::vector<Entry>		m_entries;
		bool					m_defined = false;
	};

}