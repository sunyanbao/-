#pragma once
#include "BaseInit.h"

class ParticleRenderer;
struct WindFieldData;
class ParticleRendererManager : public Renderable,private BaseInit
{
public:
	ParticleRendererManager();
	~ParticleRendererManager();
public:
	void Reset();
	void GenerateParticleRender(WindFieldData* data);
	virtual void Render(RenderParameter& param);
	void SetBox(const AxisAlignedBox3d& box);
	void SetMaxParCount(int count)
	{
		max_count_ = count;
	}
	void SetRatio(float ratio)
	{
		ratio_ =ratio;
	}
	void SetAge(int age)
	{
		age_ = age;
	}
private:
	void GenerateParticleRenderImpl(int count);
private:
	std::vector<ParticleRenderer*> renderer_list_;
	int max_count_;
	AxisAlignedBox3d box_;
	WindFieldData* data_;
	float ratio_;
	int age_;
};

