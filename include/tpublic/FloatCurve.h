#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	class FloatCurve
	{
	public:
		struct Point
		{
			void
			ToStream(
				IWriter*		aWriter) const
			{
				aWriter->WriteUInt(m_x);
				aWriter->WriteFloat(m_y);
			}

			bool
			FromStream(
				IReader*		aReader)
			{
				if (!aReader->ReadUInt(m_x))
					return false;
				if (!aReader->ReadFloat(m_y))
					return false;
				return true;
			}

			// Public data
			uint32_t			m_x = 0;
			float				m_y = 0.0f;
		};

		FloatCurve()
		{

		}

		FloatCurve(
			const SourceNode*	aSource)
		{
			aSource->GetArray()->ForEachChild([&](
				const SourceNode* aChild)
			{
				TP_VERIFY(aChild->m_type == SourceNode::TYPE_ARRAY && aChild->m_children.size() == 2, aChild->m_debugInfo, "Not a valid curve point.");
				AddPoint({ aChild->m_children[0]->GetUInt32(), aChild->m_children[1]->GetFloat() });
			});
		}

		void
		ToStream(
			IWriter*			aWriter) const
		{
			aWriter->WriteObjects(m_points);
		}

		bool
		FromStream(
			IReader*			aReader)
		{
			if (!aReader->ReadObjects(m_points))
				return false;
			return true;
		}

		float
		Sample(
			uint32_t			aX) const
		{
			if(m_points.empty())
				return 0.0f;
			
			if(m_points.size() == 1)
				return m_points[0].m_y;
			
			if (aX <= m_points[0].m_x)
				return m_points[0].m_y;
			
			if (aX >= m_points[m_points.size() - 1].m_x)
				return m_points[m_points.size() - 1].m_y;
			
			const Point* a = NULL;
			const Point* b = NULL;

			if (m_points.size() == 2)
			{
				a = &m_points[0];
				b = &m_points[1];
			}
			else
			{
				size_t searchMin = 0;
				size_t searchMax = m_points.size();

				while(searchMin < searchMax - 1)
				{
					assert(searchMax > searchMin);
					size_t i = (searchMin + searchMax) / 2;
					assert(i < m_points.size());
					const Point& p = m_points[i];

					if (aX < p.m_x)
						searchMax = i;
					else if (aX > p.m_x)
						searchMin = i;
					else
						return p.m_y;
				}

				assert(searchMin == searchMax - 1);

				a = &m_points[searchMin];
				b = &m_points[searchMax];
			}
			
			if(a->m_y < b->m_y)
				return a->m_y + ((b->m_y - a->m_y) * (float)(aX - a->m_x)) / (float)(b->m_x - a->m_x);

			return a->m_y - ((a->m_y - b->m_y) * (float)(aX - a->m_x)) / (float)(b->m_x - a->m_x);
		}

		void
		AddPoint(
			const Point&		aPoint)
		{
			if(m_points.empty())
			{
				m_points.push_back(aPoint);
			}
			else
			{
				size_t insertOffset = m_points.size();
				while(insertOffset > 0 && m_points[insertOffset - 1].m_x > aPoint.m_x)
					insertOffset--;

				m_points.insert(m_points.begin() + insertOffset, aPoint);
			}			
		}

	private:

		std::vector<Point>		m_points;
	};

}