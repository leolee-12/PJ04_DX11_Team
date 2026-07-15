#include "MaterialEx.h"
#include "GameInstance.h"

#include <filesystem>

using namespace std::filesystem;

NS_BEGIN(Engine)

HRESULT CMaterialEx::Initialize(const MATERIAL_DATA& data)
{
	m_pProxy = CGameInstance::GetProxy();
	if (nullptr == m_pProxy)
		return E_FAIL;

	for (_uint i = 0; i < MTEX_TYPE_MAX; ++i)
	{
		for (const _string& strTextureName : data.TextureNames[i])
		{
			const _wstring strLookupName = StrToWstr(strTextureName);
			if (strLookupName.empty())
			{
#ifdef _DEBUG
				OutputDebugStringA("[TextureHub] CMaterialEx invalid texture name: ");
				OutputDebugStringA(strTextureName.c_str());
				OutputDebugStringA("\n");
#endif
				return E_FAIL;
			}

			TEXTURE_HANDLE Handle = INVALID_TEXTURE_HANDLE;
			if (FAILED(m_pProxy->Get_TextureFromHub(strLookupName.c_str(), &Handle)))
			{
#ifdef _DEBUG
				OutputDebugStringA("[TextureHub] CMaterialEx texture lookup failed: ");
				OutputDebugStringA(strTextureName.c_str());
				OutputDebugStringA("\n");
#endif
				return E_FAIL;
			}

			m_MaterialHandles[i].push_back(Handle);
		}
	}

	return S_OK;
}

HRESULT CMaterialEx::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, MTEX_TYPE eTexType, _uint iIndex)
{
	if (nullptr == pShader || nullptr == pConstantName || nullptr == m_pProxy)
		return E_FAIL;

	const _uint iType = ETOUI(eTexType);
	if (iType >= MTEX_TYPE_MAX || iIndex >= m_MaterialHandles[iType].size())
		return E_FAIL;

	return m_pProxy->Bind_TextureFromHub(
		pShader,
		pConstantName,
		m_MaterialHandles[iType][iIndex]);
}

_uint CMaterialEx::Get_TextureCount(MTEX_TYPE eType) const
{
	const _uint iType = ETOUI(eType);
	return (iType < MTEX_TYPE_MAX)
		? static_cast<_uint>(m_MaterialHandles[iType].size())
		: 0u;
}

CMaterialEx* CMaterialEx::Create(const MATERIAL_DATA& data)
{
	CMaterialEx* pInstance = new CMaterialEx();

	if (FAILED(pInstance->Initialize(data)))
	{
		MSG_BOX("Failed to Created : CMaterialEx");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMaterialEx::Free()
{
	for (auto& Handles : m_MaterialHandles)
		Handles.clear();

	Safe_Release(m_pProxy);

	__super::Free();
}

NS_END
