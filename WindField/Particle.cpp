#include "stdafx.h"
#include "Particle.h"
#include "GetData.h"

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

DeepEye::Vector3d Particle::CaclNextFramePos(float interval,const Vector2f& speed,float par_len,LonLatAlt & next_par_pos)
{
	GeographicCoordinate gc;
	gc.SetOrigin(lla_pos_);
	Vector3f lcl = Vector3f::s_zero;
	Vector3f next_par_pos_temp = Vector3f::s_zero;
	//lcl.x = speed.x*interval/1000.0f;
	//lcl.z = speed.y*interval/1000.0f;
	lcl.x = speed.x*GetRatio(speed);//interval控制速度比例值
	lcl.z = speed.y*GetRatio(speed);

	CDEK_Vector3f unitlcl = CDEK_Vector3f::s_zero;
	DEK_Vector3Normalize(unitlcl,lcl);
	par_len *= 1000;
	next_par_pos_temp.x = unitlcl.x*par_len;
	next_par_pos_temp.z = unitlcl.z*par_len;
	LonLatAlt next_pos;
	gc.LocalToLonLatAlt(next_pos,lcl);
	gc.LocalToLonLatAlt(next_par_pos,next_par_pos_temp);
	return next_pos;
}

float Particle::GetRatio(const Vector2f& speed)
{
	float value=0;
	value = sqrt(pow(speed.x,2)-pow(speed.y,2));
	float ratio=0;
	map<float,float>::iterator it = GetData::GetInstance()->speed_ratio_map_.begin();
	for (;it!=GetData::GetInstance()->speed_ratio_map_.end();it++)
	{
		if (value>=it->first)
		{
			ratio=it->second;
		}
	}
	return ratio;
}