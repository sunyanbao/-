#include "stdafx.h"
#include "ParticleRenderer.h"
#include "Particle.h"
#include "WindFieldData.h"

ParticleRenderer::ParticleRenderer(Particle* first_particle,WindFieldData* data,const Vector3d& llh_origin)
{
	particle_list_.push_back(first_particle);
	wind_field_data_ = data;
	LonLatAltToWGS(wgs_origin_,llh_origin);
	age_ = 2;
	CString path = resource_path_ + "plant/WindField2/wind.png";
	MixedFileSystem::getSingleton().makeSureFileExistAndLatest(path.GetString());
	file_texture_ = framework_->m_fileTextureManager->GetFileTexture(path);
	ratio_ = 1;
}

ParticleRenderer::~ParticleRenderer()
{
	for (size_t i = 0;i!=particle_list_.size();++i)
	{
		SafeDeleteSetNull(particle_list_[i]);
	}
	SafeReleaseSetNull(file_texture_);
}

void ParticleRenderer::Render(RenderParameter& param)
{
	if (particle_list_.empty())
	{
		return;
	}
	assert(!particle_list_.empty());
	if(!RenderUsageIsScene(param.usage))
	{
		return;
	}
	if (!MakeNewParticle(param))
	{
		return;
	}
	if (vertices_.size() <= 1)
	{
		return;
	}
	size_t count = vertices_.size();
	indices_.clear();
	indices_.reserve(2*count-2);
	for (size_t i = 1 ;i!= vertices_.size();++i)
	{
		indices_.push_back(i-1);
		indices_.push_back(i);
	}
	Vector3f sceneOrigin = param.sceneManager->GetSceneOrigin();
	Vector3f translation;
	Vector3Subtract(translation, wgs_origin_, sceneOrigin);
	Matrix4x4f world_matrix;
	Matrix4x4Translation(world_matrix, translation.x, translation.y, translation.z);
	Canvas* canvas = framework_->m_canvas;
	canvas->DrawLineList(&vertices_[0],vertices_.size(),&indices_[0],indices_.size(),file_texture_->GetTexture_(true),&world_matrix);
	//canvas->DrawLineList(&vertices_[0],vertices_.size(),&indices_[0],indices_.size(),&world_matrix);
	//canvas->DrawLineStrip(&vertices_[0],vertices_.size(),&world_matrix);
}

bool ParticleRenderer::MakeNewParticle(const RenderParameter& param)
{
	if (age_ > 1)
	{
		Particle* p = particle_list_.back();
		if (p == NULL)
		{
			return false;
		}
		Vector2f speed = Vector2f::s_zero;
		bool is_suc = wind_field_data_->CalcWindSpeed(p->lla_pos_,speed);
		if (!is_suc)
		{
			age_ = 1;
			return false;
		}
		if (abs(speed.x) < 1e-6 || abs(speed.y) < 1e-6)
		{
			age_ = 1;
			return false;
		}
		Vector3d next_pos = p->CaclNextFramePos(param.interval,speed,ratio_);
		Particle* new_particle = new Particle(next_pos,0/*p->age_*/);
		--age_;

		{
			Vector3d wgs_pos;
			LonLatAltToWGS(wgs_pos,p->lla_pos_);
			FVFXyzTex vertex;
			//FVFXyz vertex;
			Vector3Subtract(vertex.position, wgs_pos, wgs_origin_);
			vertices_.push_back(vertex);
			for (size_t i = 0;i!=vertices_.size();++i)
			{
				vertices_[i].texcoord.y = 0;
				vertices_[i].texcoord.x = i/(float)vertices_.size();
			}
		}

		particle_list_.push_back(new_particle);
	}
	else
	{
		if (vertices_.empty())
		{
			age_ = 0;
			return false;
		}
		vertices_.erase(vertices_.begin());
		if (vertices_.empty())
		{
			age_ = 0;
			return false;
		}
		for (size_t i = 0;i!=vertices_.size();++i)
		{
			vertices_[i].texcoord.y = 0;
			vertices_[i].texcoord.x = i/(float)vertices_.size();
		}
	}
	
	return true;
}
