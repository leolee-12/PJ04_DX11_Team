#pragma once

#include "Component.h"

NS_BEGIN(Engine)

class ENGINE_DLL CTexture final : public CComponent
{
private:
	CTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CTexture(const CTexture& Prototype);
	virtual ~CTexture() = default;

public:
	virtual HRESULT Initialize_Prototype(const _tchar* pTextureFilePath, _uint iNumTextures, _bool bCacheCpu);
	virtual HRESULT Initialize(void* pArg) override;

public:
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, _uint iTextureIndex);
	_ubyte  Sample_Alpha(_uint iTextureIndex, _float u, _float v);

public:
	void Get_TextureSize(_uint iIndex, _uint* pOutWidth, _uint* pOutHeight) const;
	void Get_TextureSize(_uint iIndex, _float2* pOutSize) const;
	_uint Get_NumTextures() const { return m_iNumTextures;  }
	_uint Get_ArraySize() const;		// Texture2DArray 슬라이스 개수
private:
	struct FCpuImage {
		vector<_ubyte> pixels;
		size_t rowPitch = 0;
		_uint width = 0, height = 0;
	};

private:
	_uint											m_iNumTextures = {};
	vector<ID3D11ShaderResourceView*>				m_Textures;

	_bool               m_bCacheCpu = false;
	vector<FCpuImage>   m_Images;

public:
	static CTexture* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pTextureFilePath, _uint iNumTextures, _bool bCacheCpu = false);
	virtual CComponent* Clone(void* pArg) override;
	virtual void Free() override;
};

NS_END