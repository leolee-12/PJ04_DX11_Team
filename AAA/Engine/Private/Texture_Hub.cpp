#include "Texture_Hub.h"
#include "Shader.h"

#include <cwctype>
#include <filesystem>
#include <mutex>

namespace
{
	using namespace std::filesystem;

	wstring To_NormalizedTextureKey(const _tchar* pTexturePath)
	{
		if (nullptr == pTexturePath || 0 == pTexturePath[0])
			return {};

		error_code ec;
		path ConcretePath(pTexturePath);
		path NormalizedPath = weakly_canonical(ConcretePath, ec);
		if (ec)
		{
			ec.clear();
			NormalizedPath = absolute(ConcretePath, ec);
			if (ec)
				return {};

			NormalizedPath = NormalizedPath.lexically_normal();
		}

		wstring strKey = NormalizedPath.generic_wstring();
		transform(strKey.begin(), strKey.end(), strKey.begin(),
			[](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });
		return strKey;
	}

	HRESULT Create_TextureSRV(
		ID3D11Device* pDevice,
		const _tchar* pTexturePath,
		ID3D11ShaderResourceView** ppOutSRV)
	{
		if (nullptr == pDevice || nullptr == pTexturePath || nullptr == ppOutSRV)
			return E_FAIL;

		*ppOutSRV = nullptr;

		path TexturePath(pTexturePath);
		wstring strExtension = TexturePath.extension().wstring();
		transform(strExtension.begin(), strExtension.end(), strExtension.begin(),
			[](wchar_t ch) { return static_cast<wchar_t>(towlower(ch)); });

		if (L".dds" == strExtension)
			return CreateDDSTextureFromFile(pDevice, pTexturePath, nullptr, ppOutSRV);

		return CreateWICTextureFromFile(pDevice, pTexturePath, nullptr, ppOutSRV);
	}
}

NS_BEGIN(Engine)

CTexture_Hub::CTexture_Hub(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice{ pDevice }
	, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CTexture_Hub::LoadOrGet(const _tchar* pTexturePath, TEXTURE_HANDLE* pOut)
{
	if (nullptr == pOut)
		return E_FAIL;

	*pOut = INVALID_TEXTURE_HANDLE;

	if (nullptr == pTexturePath || 0 == pTexturePath[0] || nullptr == m_pDevice)
		return E_FAIL;

	const wstring strNormalizedPath = To_NormalizedTextureKey(pTexturePath);
	if (strNormalizedPath.empty())
		return E_FAIL;

	unique_lock<shared_mutex> Lock(m_Mutex);

	const auto iter = m_HandleByNormalizedPath.find(strNormalizedPath);
	if (iter != m_HandleByNormalizedPath.end())
	{
		++m_iCacheHitCount;
		*pOut = iter->second;
		return S_OK;
	}

	++m_iCacheMissCount;

	ID3D11ShaderResourceView* pSRV = nullptr;
	const HRESULT hr = Create_TextureSRV(m_pDevice, pTexturePath, &pSRV);
	if (FAILED(hr))
	{
		++m_iLoadFailureCount;
		return hr;
	}

	try
	{
		const TEXTURE_HANDLE Handle = static_cast<TEXTURE_HANDLE>(m_SRVs.size());
		m_SRVs.push_back(pSRV);
		m_HandleByNormalizedPath.emplace(strNormalizedPath, Handle);
		*pOut = Handle;
	}
	catch (...)
	{
		Safe_Release(pSRV);
		return E_FAIL;
	}

	return S_OK;
}

HRESULT CTexture_Hub::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, TEXTURE_HANDLE Handle) const
{
	if (nullptr == pShader || nullptr == pConstantName)
		return E_FAIL;

	ID3D11ShaderResourceView* pSRV = Get_SRV(Handle);
	if (nullptr == pSRV)
		return E_FAIL;

	return pShader->Bind_SRV(pConstantName, pSRV);
}

_bool CTexture_Hub::Is_Valid(TEXTURE_HANDLE Handle) const
{
	shared_lock<shared_mutex> Lock(m_Mutex);
	return Handle < m_SRVs.size() && nullptr != m_SRVs[Handle];
}

_bool CTexture_Hub::Is_CompatibleDevice(ID3D11Device* pDevice) const
{
	return nullptr != pDevice && pDevice == m_pDevice;
}

CTexture_Hub::TEXTURE_HUB_STATS CTexture_Hub::Get_Stats() const
{
	shared_lock<shared_mutex> Lock(m_Mutex);

	TEXTURE_HUB_STATS Stats{};
	Stats.iUniqueTextureCount = static_cast<_uint>(m_SRVs.size());
	Stats.iCacheHitCount = m_iCacheHitCount;
	Stats.iCacheMissCount = m_iCacheMissCount;
	Stats.iLoadFailureCount = m_iLoadFailureCount;
	return Stats;
}

ID3D11ShaderResourceView* CTexture_Hub::Get_SRV(TEXTURE_HANDLE Handle) const
{
	shared_lock<shared_mutex> Lock(m_Mutex);
	if (Handle >= m_SRVs.size())
		return nullptr;

	return m_SRVs[Handle];
}

CTexture_Hub* CTexture_Hub::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CTexture_Hub* pInstance = new CTexture_Hub(pDevice, pContext);

	if (nullptr == pInstance)
	{
		MSG_BOX("Failed to Created : CTexture_Hub");
	}

	return pInstance;
}


void CTexture_Hub::Free()
{
	__super::Free();

	for (auto& pSRV : m_SRVs)
		Safe_Release(pSRV);
	m_SRVs.clear();
	m_HandleByNormalizedPath.clear();

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}

NS_END
