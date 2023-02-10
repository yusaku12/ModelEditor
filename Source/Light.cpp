#include "Light.h"
#include "imgui.h"
#include <filesystem>

#pragma region LIGHTING

LIGHTING::LIGHTING(L_TYPE t) : type(t)
{
	if (type == L_TYPE::DIRECTIONAL)return;
	//d_Primitive = std::make_shared<DEBUG_SPHERE>();
}

LIGHTING::L_TYPE LIGHTING::Type()
{
	return type;
}

void LIGHTING::RenderDebug()
{
	//if (!d_Primitive)
	//	return;
	//d_Primitive->SetPosition(position);
	//d_Primitive->Execute();
	//d_Primitive->Render({ 1.0f, 0.0f, 0.0f, 1.0f });
}

#pragma region Setters

void LIGHTING::SetDirection(XMFLOAT3 dir)
{
	direction = dir;
}
void LIGHTING::SetPosition(XMFLOAT3 pos)
{
	position = pos;
}
void LIGHTING::SetColour(XMFLOAT4 col)
{
	colour = col;
}
void LIGHTING::SetInnerCorner(float in)
{
	inner = inner;
}
void LIGHTING::SetOuterCorner(float out)
{
	outer = outer;
}
void LIGHTING::SetRange(float r)
{
	range = r;
}
void LIGHTING::SetType(L_TYPE t)
{
	type = t;
}

#pragma endregion
#pragma region Getters

XMFLOAT3 LIGHTING::Direction()
{
	return direction;
}
XMFLOAT3 LIGHTING::Position()
{
	return position;
}
XMFLOAT4 LIGHTING::Colour()
{
	return colour;
}
float LIGHTING::Range()
{
	return range;
}
float LIGHTING::Inner()
{
	return inner;
}
float LIGHTING::Outer()
{
	return outer;
}

#pragma endregion

#pragma endregion
#pragma region LIGHTINGMANAGER

void LightingManager::Insert(std::string n, std::shared_ptr<LIGHTING> l)
{
	dataset.insert(std::make_pair(n, l));
}
void LightingManager::Remove(std::string n)
{
	for (auto d = dataset.begin(); d != dataset.end(); ++d)
	{
		if (n == d->first)
			dataset.erase(d);
	}
}
void LightingManager::Create(std::string n, LIGHTING::L_TYPE t)
{
	std::shared_ptr<LIGHTING>l = std::make_shared<LIGHTING>(t);
	Insert(n, l);
}
void LightingManager::RenderUI()
{
	ImGui::Begin("Stage Settings");

	static std::shared_ptr<LIGHTING>d{ dataset.begin()->second };
	if (ImGui::BeginCombo("Lights : ", Name(d).c_str()))
	{
		for (auto& dt : dataset)
		{
			bool selected{ Name(d) == dt.first };
			if (ImGui::Selectable(dt.first.c_str(), &selected))
				d = dt.second;
		}
		ImGui::EndCombo();
	}

	if (ImGui::TreeNode(Name(d).c_str()))
	{
		switch (d->Type()) {
		case LIGHTING::L_TYPE::DIRECTIONAL: {
			ImGui::DragFloat3("Direction : ", &d->direction.x, 0.05f, -10.0f, 10.0f);
			ImGui::ColorEdit4("Colour : ", &d->colour.x);
			break;
		}
		}
		ImGui::TreePop();
	}
	ImGui::End();
}
void LightingManager::OutputFile(std::string f)
{
	//std::filesystem::path path(f);
	//if (!std::filesystem::exists(path))
	//	assert("File does not exist");
	//std::ofstream ofs(f);
	//cereal::BinaryOutputArchive output(ofs);
	//output(dataset);
}
void LightingManager::ReadFromFile(std::string f)
{
	//std::filesystem::path path(f);
	//if (!std::filesystem::exists(path))
	//	assert("File does not exist");
	//std::ifstream ifs(f);
	//cereal::BinaryInputArchive in(ifs);
	//in(dataset);
}
void LightingManager::RenderDebug()
{
}
std::string LightingManager::Name(std::shared_ptr<LIGHTING>d)
{
	for (auto& dt : dataset)
		if (dt.second == d)
			return dt.first;
	assert(!"No such light");
	return "";
}
std::shared_ptr<LIGHTING> LightingManager::Retrieve(std::string n)
{
	return dataset.find(n)->second;
}
#pragma endregion