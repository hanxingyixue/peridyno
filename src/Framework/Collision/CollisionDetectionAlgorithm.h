#pragma once

#include "CollisionData.h"

namespace dyno
{
	template<typename Real>
	class CollisionDetection
	{
	public:
		using Quat1f = Quat<Real>;
		using Coord3D = Vector<Real, 3>;
		using Matrix3D = SquareMatrix<Real, 3>;
		using Transform3D = Transform<Real, 3>;
		using Manifold = TManifold<Real>;
		using OBox3D = TOrientedBox3D<Real>;

		//--------------------------------------------------------------------------------------------------
		// Resources:
		// http://www.randygaul.net/2014/05/22/deriving-obb-to-obb-intersection-sat/
		// https://box2d.googlecode.com/files/GDC2007_ErinCatto.zip
		// https://box2d.googlecode.com/files/Box2D_Lite.zip
		DYN_FUNC static void request(Manifold& m, OBox3D box0, OBox3D box1);

	private:

	};

}

#include "CollisionDetectionAlgorithm.inl"