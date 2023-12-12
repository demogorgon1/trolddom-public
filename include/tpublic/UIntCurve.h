#pragma once

#include "IReader.h"
#include "IWriter.h"
#include "SourceNode.h"

namespace tpublic
{

	template <typename _T>
	class UIntCurve
	{
	public:
		struct Point
		{
			void
			ToStream(
				IWriter*		aWriter) const
			{
				aWriter->WriteUInt(m_x);
				aWriter->WriteUInt(m_y);
			}

			bool
			FromStream(
				IReader*		aReader)
			{
				if (!aReader->ReadUInt(m_x))
					return false;
				if (!aReader->ReadUInt(m_y))
					return false;
				return true;
			}

			// Public data
			_T					m_x = _T(0);
			_T					m_y = _T(0);
		};

		UIntCurve()
		{

		}

		UIntCurve(
			const SourceNode*	aSource)
		{
			aSource->GetArray()->ForEachChild([&](
				const SourceNode* aChild)
			{
				TP_VERIFY(aChild->m_type == SourceNode::TYPE_ARRAY && aChild->m_children.size() == 2, aChild->m_debugInfo, "Not a valid curve point.");
				AddPoint({ (_T)aChild->m_children[0]->GetUInt64(), (_T)aChild->m_children[1]->GetUInt64() });
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

		_T
		Sample(
			_T					aX) const
		{
			if(m_points.empty())
				return _T(0);
			
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
				return a->m_y + ((b->m_y - a->m_y) * (aX - a->m_x)) / (b->m_x - a->m_x);

			return a->m_y - ((a->m_y - b->m_y) * (aX - a->m_x)) / (b->m_x - a->m_x);
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