#pragma once

class Particle
{
public:
	Particle();
	Particle(const Vector3d& pos,int age);
public:
	Vector3d CaclNextFramePos(float interval,const Vector2f& speed,float par_len,LonLatAlt & next_par_pos);
private:
	float GetRatio(const Vector2f& speed);
public:
	Vector3d lla_pos_;
	//int age_;
};
