#include "Material.h"
#include "Shader.h"

CMaterial::CMaterial(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: m_pDevice { pDevice }
	, m_pContext{ pContext }
{
	Safe_AddRef(m_pDevice);
	Safe_AddRef(m_pContext);
}

HRESULT CMaterial::Initialize(const MATERIAL_DATA& data, const _char* pModelFilePath)
{
	_char			szDrive[MAX_PATH] = {};
	_char			szDir[MAX_PATH] = {};

	_splitpath_s(pModelFilePath, szDrive, MAX_PATH, szDir, MAX_PATH, nullptr, 0, nullptr, 0);

	for (size_t i = 0; i < MTEX_TYPE_MAX; ++i)
	{
		for (const string& strTextureName : data.TextureNames[i])
		{
			string strBaseName = strTextureName;
			if (size_t iDot = strBaseName.rfind('.'); iDot != string::npos)
				strBaseName = strBaseName.substr(0, iDot);

			const string strFolder = string(szDrive) + string(szDir);
			const string strDDS = strFolder + strBaseName + ".dds";
			const string strPNG = strFolder + strBaseName + ".png";

			ID3D11ShaderResourceView* pSRV = { nullptr };
			HRESULT hr = E_FAIL;

			// 같은 폴더에서 .dds 우선, 없으면 .png
			if (std::filesystem::exists(strDDS))
				hr = CreateDDSTextureFromFile(m_pDevice, StrToWstr(strDDS).c_str(), nullptr, &pSRV);
			else if (std::filesystem::exists(strPNG))
				hr = CreateWICTextureFromFile(m_pDevice, StrToWstr(strPNG).c_str(), nullptr, &pSRV);

			if (FAILED(hr))
				return E_FAIL;

			m_Materials[i].push_back(pSRV);
		}
	}

	return S_OK;
}

HRESULT CMaterial::Bind_ShaderResource(CShader* pShader, const _char* pConstantName, MTEX_TYPE eTexType, _uint iIndex)
{
	const _uint eType = ETOUI(eTexType);

	if (eType >= MTEX_TYPE_MAX ||
		iIndex >= m_Materials[eType].size())
		return E_FAIL;

	return pShader->Bind_SRV(pConstantName, m_Materials[eType][iIndex]);
}

CMaterial* CMaterial::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const MATERIAL_DATA& data, const _char* pModelFilePath)
{
	CMaterial* pInstance = new CMaterial(pDevice, pContext);

	if (FAILED(pInstance->Initialize(data, pModelFilePath)))
	{
		MSG_BOX("Failed to Created : CMaterial");
		Safe_Release(pInstance);
	}

	return pInstance;
}

void CMaterial::Free()
{
	__super::Free();

	for (auto& SRVs : m_Materials)
	{
		for (auto& pSRV : SRVs)
			Safe_Release(pSRV);
		SRVs.clear();
	}

	Safe_Release(m_pDevice);
	Safe_Release(m_pContext);
}
