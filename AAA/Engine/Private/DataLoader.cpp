#include "DataLoader.h"
#include <fstream>
#include <sstream>

static string ReadStr(ifstream& f)
{
	uint32_t len;
	f.read(reinterpret_cast<char*>(&len), sizeof(uint32_t));
	string s(len, '\0');
	if (len > 0) f.read(s.data(), len);
	return s;
}

template<typename T>
static void ReadVal(ifstream& f, T& v)
{
	f.read(reinterpret_cast<char*>(&v), sizeof(T));
}

HRESULT CDataLoader::Read_Json(const _tchar* strFilePath, string* pOutContnet)
{
    ifstream file(strFilePath, ios::binary);
    if (!file.is_open())
        return E_FAIL;

    file.seekg(0, ios::end);
    const streamsize iSize = file.tellg();
    file.seekg(0, ios::beg);

    pOutContnet->resize(static_cast<size_t>(iSize));
    file.read(pOutContnet->data(), iSize);
    return S_OK;
}

void CDataLoader::Parse_Float4(const json& jArray, _float4* pOutFloat4)
{
	if (!jArray.is_array() || jArray.size() != 4)
		return;

	pOutFloat4->x = jArray[0].get<float>();
	pOutFloat4->y = jArray[1].get<float>();
	pOutFloat4->z = jArray[2].get<float>();
	pOutFloat4->w = jArray[3].get<float>();
}

HRESULT CDataLoader::Read_ysh(const _tchar* pFilePath, MODEL_DATA& out)
{
    ifstream file(pFilePath, ios::binary);
    if (!file.is_open()) 
        return E_FAIL;

    uint32_t magic, version, type;
    ReadVal(file, magic);
    ReadVal(file, version);
    ReadVal(file, type);

    if (magic != 0x2E595348) return E_FAIL;

    out.eType = (type == 0) ? MODEL::NONANIM
        : (type == 2) ? MODEL::MAP
        : MODEL::ANIM;

    // 본
    if (out.eType == MODEL::ANIM)
        Read_BoneHierarchy(file, out.Bones, -1);

    // 메시
    uint32_t numMeshes;
    ReadVal(file, numMeshes);
    for (uint32_t i = 0; i < numMeshes; ++i)
    {
        MESH_DATA mesh;
        mesh.strName = ReadStr(file);
        ReadVal(file, mesh.iMaterialIndex);

        uint32_t numVerts;
        ReadVal(file, numVerts);

        if (out.eType == MODEL::NONANIM)
        {
            mesh.NonAnimVertices.resize(numVerts);
            file.read(reinterpret_cast<char*>(mesh.NonAnimVertices.data()),
                sizeof(VTXMESH_DATA) * numVerts);
        }
        else if (out.eType == MODEL::MAP)
        {
            mesh.MapVertices.resize(numVerts);
            file.read(reinterpret_cast<char*>(mesh.MapVertices.data()),
                sizeof(VTXMAPMESH_DATA) * numVerts);
        }
        else
        {
            mesh.AnimVertices.resize(numVerts);
            file.read(reinterpret_cast<char*>(mesh.AnimVertices.data()),
                sizeof(VTXANIMMESH_DATA) * numVerts);
        }

        uint32_t numIndices;
        ReadVal(file, numIndices);
        mesh.Indices.resize(numIndices);
        file.read(reinterpret_cast<char*>(mesh.Indices.data()),
            sizeof(uint32_t) * numIndices);

        // 메시 영향 본
        if (out.eType == MODEL::ANIM)
        {
            uint32_t numBones;
            ReadVal(file, numBones);
            for (uint32_t j = 0; j < numBones; ++j)
            {
                MESH_BONE_DATA bone;
                bone.strName = ReadStr(file);
                ReadVal(file, bone.OffsetMatrix);
                mesh.Bones.push_back(move(bone));
            }
        }

        out.Meshes.push_back(move(mesh));
    }

    // 머테리얼
    uint32_t numMaterials;
    ReadVal(file, numMaterials);
    for (uint32_t i = 0; i < numMaterials; ++i)
    {
        MATERIAL_DATA mat;
        for (uint32_t k = 0; k < MTEX_TYPE_MAX; ++k)
        {
            uint32_t numTex;
            ReadVal(file, numTex);
            for (uint32_t j = 0; j < numTex; ++j)
                mat.TextureNames[k].push_back(ReadStr(file));
        }
        out.Materials.push_back(move(mat));
    }

    // 애니메이션
    if (out.eType == MODEL::ANIM)
    {
        uint32_t numAnims;
        ReadVal(file, numAnims);
        for (uint32_t i = 0; i < numAnims; ++i)
        {
            ANIMATION_DATA anim;
            anim.strName = ReadStr(file);
            ReadVal(file, anim.fDuration);
            ReadVal(file, anim.fTickPerSecond);

            uint32_t numChannels;
            ReadVal(file, numChannels);
            for (uint32_t j = 0; j < numChannels; ++j)
            {
                CHANNEL_DATA channel;
                channel.strBoneName = ReadStr(file);

                uint32_t numKeyFrames;
                ReadVal(file, numKeyFrames);
                channel.KeyFrames.resize(numKeyFrames);
                file.read(reinterpret_cast<char*>(channel.KeyFrames.data()),
                    sizeof(KEYFRAME_DATA) * numKeyFrames);

                anim.Channels.push_back(move(channel));
            }
            out.Animations.push_back(move(anim));
        }
    }

    return S_OK;
}

void CDataLoader::Read_BoneHierarchy(ifstream& file, vector<BONE_DATA>& bones, _int parentIdx)
{
	BONE_DATA bone;
	bone.strName = ReadStr(file);
	file.read(reinterpret_cast<char*>(&bone.TransformationMatrix), sizeof(_float4x4));
	bone.iParentIndex = parentIdx;

	bones.push_back(bone);
	_int myIdx = (_int)bones.size() - 1;

	uint32_t numChildren;
	ReadVal(file, numChildren);
	for (uint32_t i = 0; i < numChildren; ++i)
		Read_BoneHierarchy(file, bones, myIdx);
}
