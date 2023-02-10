#pragma once

#include "Singleton.h"
#include "Texture.h"
#include <map>
#include <memory>
#include <string>

class TextureManager : public Singleton<TextureManager>
{
    std::map<std::wstring, std::weak_ptr<TEXTURE>>textureMap;
    /// <summary>
    /// <para> Private Function, inserts a new texture into the map </para>
    /// <para> Private�֐��B�V�����e�L�X�`�����}�b�v�ɒ��� </para>
    /// </summary>
    /// <param name=""></param>
    std::shared_ptr<TEXTURE>Insert(std::wstring file_path);
    std::shared_ptr<TEXTURE> GenerateNormal();
    std::shared_ptr<TEXTURE> GenerateEmpty();
public:
    /// <summary>
    /// <para> Returns a texture from the map. Generates a new one if not found </para>
    /// <para> �}�b�v����e�L�X�`����߂�B������Ȃ��ꍇ�ɐV�����e�L�X�`���𐶐�</para>
    /// </summary>
    /// <returns></returns>
    std::shared_ptr<TEXTURE> Retrieve(std::wstring file_path);
};