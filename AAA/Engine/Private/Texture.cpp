#include "Texture.h"
#include "Shader.h"
#include <DirectXTex.h>

CTexture::CTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CComponent { pDevice, pContext }
{
}

CTexture::CTexture(const CTexture& Prototype)
	: CComponent(Prototype)
	, m_iNumTextures { Prototype.m_iNumTextures }
	, m_Textures { Prototype.m_Textures }
{
	for (auto& pTexture : m_Textures)
		Safe_AddRef(pTexture);
}

HRESULT CTexture::Initialize_Prototype(const _tchar* pTextureFilePath, _uint iNumTextures, _bool bCacheCpu)
{
	m_iNumTextures = iNumTextures;
	m_bCacheCpu = bCacheCpu;

	_tchar			szTextureFilePath[MAX_PATH] = TEXT("");

	for (size_t i = 0; i < iNumTextures; i++)
	{
		wsprintf(szTextureFilePath, pTextureFilePath, i);

		_tchar szEXT[MAX_PATH] = {};
		_wsplitpath_s(szTextureFilePath, nullptr, 0, nullptr, 0, nullptr, 0, szEXT, MAX_PATH);

		ID3D11ShaderResourceView* pSRV = nullptr;
		HRESULT hr = {};

		if (bCacheCpu)
		{
			DirectX::ScratchImage scratch;
			hr = DirectX::LoadFromWICFile(szTextureFilePath, DirectX::WIC_FLAGS_NONE, nullptr, scratch);
			if (FAILED(hr)) 
				return E_FAIL;

			hr = DirectX::CreateShaderResourceView(m_pDevice, scratch.GetImages(),
				scratch.GetImageCount(), scratch.GetMetadata(), &pSRV);
			if (FAILED(hr)) 
				return E_FAIL;

			const DirectX::Image* img = scratch.GetImage(0, 0, 0);
			FCpuImage cpu;
			cpu.width = img->width;
			cpu.height = img->height;
			cpu.rowPitch = img->rowPitch;
			cpu.pixels.assign(img->pixels, img->pixels + img->rowPitch * img->height);
			m_Images.push_back(move(cpu));
		}
		else
		{
			if (false == lstrcmp(szEXT, TEXT(".dds")))
				hr = CreateDDSTextureFromFile(m_pDevice, szTextureFilePath, nullptr, &pSRV);
			else if (false == lstrcmp(szEXT, TEXT(".tga")))
				hr = E_FAIL;
			else
				hr = CreateWICTextureFromFileEx(m_pDevice, szTextureFilePath,
					0, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0, 0,
					WIC_LOADER_IGNORE_SRGB,
					nullptr, &pSRV);

			if (FAILED(hr)) 
				return E_FAIL;
		}

		m_Textures.push_back(pSRV);
	}

	return S_OK;
}

_ubyte CTexture::Sample_Alpha(_uint iIdx, _float u, _float v)
{
	if (!m_bCacheCpu || iIdx >= m_Images.size()) return 255;

	auto& img = m_Images[iIdx];
	_uint px = (_uint)(u * (_float)(img.width - 1));
	_uint py = (_uint)(v * (_float)(img.height - 1));
	return img.pixels[py * img.rowPitch + px * 4 + 3];
}

void CTexture::Get_TextureSize(_uint iIndex, _uint* pOutWidth, _uint* pOutHeight) const
{
	if (iIndex >= m_iNumTextures) return;

	ID3D11Resource* pResource = nullptr;
	m_Textures[iIndex]->GetResource(&pResource);

	ID3D11Texture2D* pTex2D = nullptr;
	pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTex2D);

	D3D11_TEXTURE2D_DESC desc;
	pTex2D->GetDesc(&desc);

	*pOutWidth = desc.Width;
	*pOutHeight = desc.Height;

	pTex2D->Release();
	pResource->Release();
}

void CTexture::Get_TextureSize(_uint iIndex, _float2* pOutSize) const
{
	if (iIndex >= m_iNumTextures) return;

	ID3D11Resource* pResource = nullptr;
	m_Textures[iIndex]->GetResource(&pResource);

	ID3D11Texture2D* pTex2D = nullptr;
	pResource->QueryInterface(__uuidof(ID3D11Texture2D), (void**)&pTex2D);

	D3D11_TEXTURE2D_DESC desc;
	pTex2D->GetDesc(&desc);

	pOutSize->x = static_cast<_float>(desc.Width);
	pOutSize->y = static_cast<_float>(desc.Height);

	pTex2D->Release();
	pResource->Release();
}

HRESULT CTexture::Initialize(void* pArg)
{
	return S_OK;
}

HRESULT CTexture::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, _uint iTextureIndex)
{
	if (iTextureIndex >= m_iNumTextures)
		return E_FAIL;

	return pShader->Bind_SRV(pConstantName, m_Textures[iTextureIndex]);
}

CTexture* CTexture::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const _tchar* pTextureFilePath, _uint iNumTextures, _bool bCacheCpu)
{
	CTexture* pInstance = new CTexture(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype(pTextureFilePath, iNumTextures, bCacheCpu)))
	{
		MSG_BOX("Failed to Created : CTexture");
		Safe_Release(pInstance);
	}

	return pInstance;
}

CComponent* CTexture::Clone(void* pArg)
{
	CTexture* pInstance = new CTexture(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		MSG_BOX("Failed to Cloned : CTexture");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CTexture::Free()
{
	__super::Free();

	for (auto& pTexture : m_Textures)
		Safe_Release(pTexture);

	m_Textures.clear();
}
