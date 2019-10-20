#pragma once

class Particle
{
public:
	Particle();
	Particle(const Vector3d& pos,int age);
public:
	Vector3d CaclNextFramePos(int interval,const Vector2f& speed,float ratio);
public:
	Vector3d lla_pos_;
	//int age_;
};
