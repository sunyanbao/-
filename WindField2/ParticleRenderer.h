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

	void SetRatio(float ratio)
	{
		ratio_ =ratio;
	}
private:
	bool MakeNewParticle(const RenderParameter& param);
public:
	ParticleContainer particle_list_;
	int age_;
private:
	WindFieldData* wind_field_data_;
	Vector3d wgs_origin_;
	FileTexture* file_texture_;
private:
	std::vector<FVFXyzTex> vertices_;
	//std::vector<FVFXyz> vertices_;
	std::vector<unsigned short> indices_;
	float ratio_;
};

