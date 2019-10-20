#include "stdafx.h"
#include "Particle.h"

Particle::Particle()
{
	lla_pos_ = Vector3d::s_zero;
	//age_ = 40;
}

Particle::Particle(const Vector3d& pos,int age)
{
	 lla_pos_ = pos;
	// age_ = age;
}

DeepEye::Vector3d Particle::CaclNextFramePos(int interval,const Vector2f& speed,float ratio)
{
	GeographicCoordinate gc;
	gc.SetOrigin(lla_pos_);
	Vector3f lcl = Vector3f::s_zero;
	//lcl.x = speed.x*interval/1000.0f;
	//lcl.z = speed.y*interval/1000.0f;
	lcl.x = speed.x*interval/0.2f*ratio;
	lcl.z = speed.y*interval/0.2f*ratio;
	LonLatAlt next_pos;
	gc.LocalToLonLatAlt(next_pos,lcl);
	return next_pos;
}
