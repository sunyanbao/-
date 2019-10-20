#include "stdafx.h"
#include "ParticleRendererManager.h"
#include "WindFieldData.h"
#include "ParticleRenderer.h"
#include "Particle.h"


ParticleRendererManager::ParticleRendererManager()
{
	max_count_ = 15000;
	ratio_ = 1;
	age_ = 30;
}

ParticleRendererManager::~ParticleRendererManager()
{
	Reset();
}

void ParticleRendererManager::Reset()
{
	for (size_t i = 0;i!=renderer_list_.size();++i)
	{
		SafeDeleteSetNull(renderer_list_[i]);
	}
	renderer_list_.clear();
}

void ParticleRendererManager::GenerateParticleRender(WindFieldData* data)
{
	data_ = data;
	GenerateParticleRenderImpl(max_count_);
}

void ParticleRendererManager::Render(RenderParameter& param)
{
	for (auto it = renderer_list_.begin();it!=renderer_list_.end();)
	{
		if((*it)->age_ == 0)
		{
			SafeDeleteSetNull(*it);
			it = renderer_list_.erase(it);
		}
		else
		{
			++it;
		}
	}
	if (renderer_list_.size() < max_count_)
	{
		GenerateParticleRenderImpl(max_count_-renderer_list_.size());
	}
	Canvas* canvas = framework_->m_canvas;
	canvas->BeginDraw(param);
	canvas->SetDrawMode(Canvas::dm_disable_z);
	canvas->SetBlendMode(Canvas::bm_normal);
	canvas->SetColor(0xffffffff);
	for (size_t i = 0;i!=renderer_list_.size();++i)
	{
		renderer_list_[i]->SetRatio(ratio_);
		renderer_list_[i]->Render(param);
	}
	canvas->EndDraw();
}

void ParticleRendererManager::SetBox(const AxisAlignedBox3d& box)
{
	box_ = box;
}

void ParticleRendererManager::GenerateParticleRenderImpl(int count)
{
	while(count--)
	{
		Vector3d rand_pos = Vector3d::s_zero;
		rand_pos.x= box_.m_min.x + (box_.m_max.x-box_.m_min.x)*rand()/RAND_MAX;
		rand_pos.y= box_.m_min.y + (box_.m_max.y-box_.m_min.y)*rand()/RAND_MAX;
		int rand_age = 1 + age_*rand()/RAND_MAX;
		Particle* p = new Particle(rand_pos,rand_age);
		ParticleRenderer* renderer = new ParticleRenderer(p,data_,rand_pos);
		renderer->age_ = rand_age;
		renderer_list_.push_back(renderer);
	}
}
