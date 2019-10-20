#pragma once
#include "DeepEyeKernel/Scene/DeepEye_Renderable.h"
#include "BaseInit.h"
class Particle;
struct WindFieldData;
class ParticleRenderer : /*public Renderable,*/private BaseInit
{
	typedef std::vector<Particle*> ParticleContainer;
public:
	ParticleRenderer(Particle* first_particle,WindFieldData* data,const Vector3d& llh_origin);
	~ParticleRenderer();
public:
	void Render(RenderParameter& param);
private:
	bool MakeNewParticle(const float& interval);
public:
	ParticleContainer particle_list_;
	int age_;
	int exist_age_;
	DWORD speed_;
	float particle_len_;
	float particle_wid_;
private:
	WindFieldData* wind_field_data_;
	Vector3d wgs_origin_;
	FileTexture* file_texture_;
private:
	std::vector<FVFXyzTex> vertices_;
	//std::vector<FVFXyz> vertices_;
	std::vector<unsigned short> indices_;
};

