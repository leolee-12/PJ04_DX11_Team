#pragma once

#include "Base.h"

NS_BEGIN(Engine)

class CMaterial final : public CBase
{
private:
	CMaterial(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CMaterial() = default;

public:
	HRESULT Initialize(const MATERIAL_DATA& data, const _char* pModelFilePath);
	HRESULT Bind_ShaderResource(class CShader* pShader, const _char* pConstantName, MTEX_TYPE eTexType, _uint iIndex);

public:
	_uint Get_TextureCount(MTEX_TYPE eType) const {
		_uint i = ETOUI(eType);
		return (i < MTEX_TYPE_MAX) ? (_uint)m_Materials[i].size() : 0u;
	}

private:
	ID3D11Device* m_pDevice = { nullptr };
	ID3D11DeviceContext* m_pContext = { nullptr };
	vector<ID3D11ShaderResourceView*>			m_Materials[27];

public:
	static CMaterial* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const MATERIAL_DATA& data, const _char* pModelFilePath);
	virtual void Free() override;
};

NS_END