#include "stdafx.h"
#include "ParticleRenderer.h"
#include "Particle.h"
#include "WindFieldData.h"

ParticleRenderer::ParticleRenderer(Particle* first_particle,WindFieldData* data,const Vector3d& llh_origin)
{
	particle_list_.push_back(first_particle);
	wind_field_data_ = data;
	LonLatAltToWGS(wgs_origin_,llh_origin);
	age_ = 40;
	exist_age_ = 0;
	speed_ = 20;
	CString path = resource_path_ + "plant/WindField/wind.png";
	MixedFileSystem::getSingleton().makeSureFileExistAndLatest(path.GetString());
	file_texture_ = framework_->m_fileTextureManager->GetFileTexture(path);
	particle_len_ = 1;
	particle_wid_ = 1;
}

ParticleRenderer::~ParticleRenderer()
{
	for (size_t i = 0;i!=particle_list_.size();++i)
	{
		SafeDeleteSetNull(particle_list_[i]);
	}
	SafeReleaseSetNull(file_texture_);
	particle_list_.clear();
}

// void ParticleRenderer::Render(RenderParameter& param)
// {
// 	if (particle_list_.empty())
// 	{
// 		return;
// 	}
// 	assert(!particle_list_.empty());
// 	if(!RenderUsageIsScene(param.usage))
// 	{
// 		return;
// 	}
// 	if (!MakeNewParticle(param))
// 	{
// 		return;
// 	}
// 	if (vertices_.size() <= 1)
// 	{
// 		return;
// 	}
// 	size_t count = vertices_.size();
// 	indices_.clear();
// 	indices_.reserve(2*count-2);
// 	for (size_t i = 1 ;i!= vertices_.size();++i)
// 	{
// 		indices_.push_back(i-1);
// 		indices_.push_back(i);
// 	}
// 	Vector3f sceneOrigin = param.sceneManager->GetSceneOrigin();
// 	Vector3f translation;
// 	Vector3Subtract(translation, wgs_origin_, sceneOrigin);
// 	Matrix4x4f world_matrix;
// 	Matrix4x4Translation(world_matrix, translation.x, translation.y, translation.z);
// 	Canvas* canvas = framework_->m_canvas;
// 	canvas->DrawLineList(&vertices_[0],vertices_.size(),&indices_[0],indices_.size(),file_texture_->GetTexture_(true),&world_matrix);
// 	//canvas->DrawLineList(&vertices_[0],vertices_.size(),&indices_[0],indices_.size(),&world_matrix);
// 	//canvas->DrawLineStrip(&vertices_[0],vertices_.size(),&world_matrix);
// }

// bool ParticleRenderer::MakeNewParticle(const RenderParameter& param)
// {
// 	if (age_ > 1)
// 	{
// 		Particle* p = particle_list_.back();
// 		if (p == NULL)
// 		{
// 			return false;
// 		}
// 		Vector2f speed = Vector2f::s_zero;
// 		bool is_suc = wind_field_data_->CalcWindSpeed(p->lla_pos_,speed);
// 		if (!is_suc)
// 		{
// 			age_ = 1;
// 			return false;
// 		}
// 		if (abs(speed.x) < 1e-6 || abs(speed.y) < 1e-6)
// 		{
// 			age_ = 1;
// 			return false;
// 		}
// 		Vector3d next_pos = p->CaclNextFramePos(param.interval,speed);//下一帧的位置
// 		Particle* new_particle = new Particle(next_pos,0/*p->age_*/);
// 		--age_;
// 
// 		{
// 			Vector3d wgs_pos;
// 			LonLatAltToWGS(wgs_pos,p->lla_pos_);
// 			FVFXyzTex vertex;
// 			//FVFXyz vertex;
// 			Vector3Subtract(vertex.position, wgs_pos, wgs_origin_);
// 			vertices_.push_back(vertex);
// 			for (size_t i = 0;i!=vertices_.size();++i)
// 			{
// 				vertices_[i].texcoord.y = 0;
// 				vertices_[i].texcoord.x = i/(float)vertices_.size();
// 			}
// 		}
// 
// 		particle_list_.push_back(new_particle);
// 	}
// 	else
// 	{
// 		if (vertices_.empty())
// 		{
// 			age_ = 0;
// 			return false;
// 		}
// 		vertices_.erase(vertices_.begin());
// 		if (vertices_.empty())
// 		{
// 			age_ = 0;
// 			return false;
// 		}
// 		for (size_t i = 0;i!=vertices_.size();++i)
// 		{
// 			vertices_[i].texcoord.y = 0;
// 			vertices_[i].texcoord.x = i/(float)vertices_.size();
// 		}
// 	}
// 	
// 	return true;
// }
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

	if (!MakeNewParticle(speed_))
	{
		return;
	}

	if (vertices_.size() <= 3)
	{
		return;
	}
	size_t count = vertices_.size();
	indices_.clear();
	indices_.reserve(6);
	indices_.push_back(count-4);
	indices_.push_back(count-1);
	indices_.push_back(count-3);
	indices_.push_back(count-3);
	indices_.push_back(count-2);
	indices_.push_back(count-1);
	Vector3f sceneOrigin = param.sceneManager->GetSceneOrigin();
	Vector3f translation;
	Vector3Subtract(translation, wgs_origin_, sceneOrigin);
	Matrix4x4f world_matrix;
	Matrix4x4Translation(world_matrix, translation.x, translation.y, translation.z);
	Canvas* canvas = framework_->m_canvas;
// 	if (age_==1)
// 	{
// 		Uint32 color = canvas->GetColor();
// 		Uint32 tmp = color;
// 		tmp>>=24;
// 		tmp = tmp&0xff;
// 		CString strTemp;
// 		if (tmp>0x00)
// 		{
// 			tmp-=20;
// 			strTemp.Format("%02xffffff",tmp);
// 			sscanf(strTemp.GetBuffer(),"%08x",&tmp);
// 			canvas->SetColor(tmp);
// 		}
// 		else
// 		{
// 			age_ = 0;
// 			return;
// 		}
// 	}
	Uint32 color = 0xffffffff;
	if (exist_age_==0)//渐变处理
	{
		color = 0x22ffffff;
	}
	if (exist_age_>0)
	{
		color = 0x55ffffff;
	}
	if (exist_age_>3)
	{
		color = 0xaaffffff;
	}
	if (exist_age_>6)
	{
		color = 0xffffffff;
	}

	if (age_<=6)
	{
		color = 0xaaffffff;
	}
	if (age_<=3)
	{
		color = 0x55ffffff;
	}
	if (age_==1)
	{
		color = 0x22ffffff;
	}
	canvas->SetColor(color);
	canvas->DrawTriangleList(&vertices_[0],vertices_.size(),&indices_[0],indices_.size(),file_texture_->GetTexture_(true),&world_matrix);
}

bool ParticleRenderer::MakeNewParticle(const float& interval)
{
 	if (age_ > 0)
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
			age_ = 0;
			return false;
		}
		if (abs(speed.x) < 1e-6 || abs(speed.y) < 1e-6)
		{
			age_ = 0;//风速为0时消失
			return false;
		}
		LonLatAlt lla_par_pos;
		Vector3d next_pos = p->CaclNextFramePos(interval,speed,particle_len_,lla_par_pos);//下一帧的位置

		Particle* new_particle = new Particle(next_pos,0/*p->age_*/);
		--age_;
		exist_age_++;

		{
			Vector3d wgs_pos1,wgs_pos2;
			LonLatAltToWGS(wgs_pos1,p->lla_pos_);
			LonLatAltToWGS(wgs_pos2,lla_par_pos);
			Vector3f pos,pos_next;
			Vector3Subtract(pos,wgs_pos1,wgs_origin_);
			Vector3Subtract(pos_next,wgs_pos2,wgs_origin_);
			Vector3f v1 = pos_next-pos;
			Vector3f v2 = wgs_origin_.s_unitY;
			Vector3f v3;
			DEK_Vector3Cross(v3,v1,v2);
			CDEK_Vector3f unitV3 = CDEK_Vector3f::s_zero;
			DEK_Vector3Normalize(unitV3,v3);
			unitV3 *= particle_wid_;
			unitV3 *= 100;
			Vector3f pos1,pos2,pos3,pos4;
			pos1 = pos+unitV3;
			pos2 = pos-unitV3;
			pos3 = pos_next-unitV3;
			pos4 = pos_next+unitV3;
			FVFXyzTex vertex1,vertex2,vertex3,vertex4;
			vertex1.position = pos1;
			vertex2.position = pos2;
			vertex3.position = pos3;
			vertex4.position = pos4;

			vertex1.texcoord.x = 1;
			vertex1.texcoord.y = 0;
			vertex2.texcoord.x = 1;
			vertex2.texcoord.y = 1;
			vertex3.texcoord.x = 0;
			vertex3.texcoord.y = 1;
			vertex4.texcoord.x = 0;
			vertex4.texcoord.y = 0;

			vertices_.push_back(vertex1);
			vertices_.push_back(vertex2);
			vertices_.push_back(vertex3);
			vertices_.push_back(vertex4);
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
	}

	return true;
}
