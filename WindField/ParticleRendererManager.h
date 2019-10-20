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
	void SetMaxPaticalCount(DWORD count)
	{
		max_count_ = count;
	}
private:
	void GenerateParticleRenderImpl(int count);
private:
	std::vector<ParticleRenderer*> renderer_list_;
	int max_count_;
	AxisAlignedBox3d box_;
	WindFieldData* data_;
public:
	int age_;
	float speed_;
	float par_wid_;
	float par_len_;
};

