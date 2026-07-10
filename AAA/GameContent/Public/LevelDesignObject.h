#pragma once
#include "LevelDesign_LoadTypes.h"
#include "Editable.h"

NS_BEGIN(Client)

class CLevelDesignObject abstract : public CGameObject, public IEditable
{
	GENERATED_BODY_ABSTRACT(CLevelDesignObject)

protected:
	CLevelDesignObject(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CLevelDesignObject(const CLevelDesignObject& Prototype);
	virtual ~CLevelDesignObject() = default;

	virtual		HRESULT Initialize_Prototype() override;
	virtual		HRESULT Initialize(void* pArg) override;
	virtual		HRESULT Validate_Initialized();
	_wstring	Make_LevelDesignObjectKey() const;

public:
	const LD_OBJECT_DESC& Get_LevelDesignDesc() const { return m_tLevelDesignDesc; }

#pragma region Editable
	virtual _bool Get_EditDesc(EDITABLE_DESC* pOutDesc) const override;
	virtual HRESULT Apply_EditPolicy(const EDIT_OBJECT_POLICY& Policy) override;
	virtual const MESH_LAYER_IDX* Get_EditMeshLayer(_uint iModelSlot, _uint iMesh) const override;
	virtual HRESULT Apply_EditMeshLayer(_uint iModelSlot, _uint iMesh, const MESH_LAYER_IDX& Layer) override;
#pragma endregion

protected:
	LD_OBJECT_DESC	m_tLevelDesignDesc = {};

protected:
	void Add_EditModelSlot(vector<EDITABLE_MODEL_SLOT>* pOutSlots, const _tchar* pLabel, EDITABLE_MODEL_KIND eKind, CModel* pModel) const;
	virtual void Build_EditCapabilities(_uint* pOutCaps, EDIT_OBJECT_POLICY* pOutPolicy) const;
	virtual void Collect_EditModelSlots(vector<EDITABLE_MODEL_SLOT>* pOutSlots) const;
	virtual HRESULT On_ApplyEditPolicy(const EDIT_OBJECT_POLICY& Policy);

protected:
	virtual void Free() override;
};

NS_END