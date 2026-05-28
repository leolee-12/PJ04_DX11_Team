#pragma once
#include "Editor_Defines.h"
#include "Base.h"

NS_BEGIN(Engine)
class CShader;
NS_END


NS_BEGIN(Editor)

class CEdit_Grid final : public CBase
{
public:
    struct VTXGRID
    {
        _float3 vPosition;
        _float4 vColor;
    };

private:
    CEdit_Grid(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
    ~CEdit_Grid() = default;

public:
    void Render(const _float4x4* pView, const _float4x4* pProj);


private:
    ID3D11Device* m_pDevice = { nullptr };
    ID3D11DeviceContext* m_pContext = { nullptr };

    ID3D11Buffer* m_pVB = { nullptr };
    _uint  m_iVertexCount = {};

    Engine::CShader* m_pShader = { nullptr };

    static const D3D11_INPUT_ELEMENT_DESC s_Elements[2];

private:
    HRESULT Initialize(_uint iCellCont, _float fCellSize);
    void    Build_Vertices(_uint iCellCount, _float fCellSize);

public:
    static CEdit_Grid* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, _uint iCellCount = 100, _float fCellSize = 1.f);
    virtual void Free() override;
};

NS_END

