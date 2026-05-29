// FBX_Converter.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//

#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>
#include <filesystem>
#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <nlohmann/json.hpp>
#include <DirectXMath.h>
#include "Engine_Defines.h"

using namespace DirectX;

using json = nlohmann::ordered_json;

namespace fs = std::filesystem;

using namespace Assimp;
using namespace std;
using namespace Engine;

void Convert_NonAnim();
void Convert_Anim();
const char* GetTextureTypeName(aiTextureType eType);
void Ready_Bones(const aiNode* pAINode, json& jNode);

void New_Convert_NoAnim();
void New_Convert_Anim();
void New_Ready_Bones(ofstream& f, const aiNode* pAINode);

void PrintParentBone(aiNode* node, const std::string& targetBoneName);

void Convert_NaviMesh();
void New_Convert_Map();

void Collect_BoneList(const aiNode* pAINode, _int parentIdx, json& jBoneArray);

static const uint32_t YSH_MAGIC = 0x2E595348;
static const uint32_t YSH_VERSION = 1;

static void WriteStr(ofstream& f, const string& s) {
    uint32_t len = (uint32_t)s.size();
    f.write(reinterpret_cast<const char*>(&len), 4);
    f.write(s.data(), len);
}
template<typename T>
static void WriteVal(ofstream& f, const T& v) {
    f.write(reinterpret_cast<const char*>(&v), sizeof(T));
}

int main()
{	
    BONE_DATA;
    New_Convert_NoAnim();

    New_Convert_Anim();

    Convert_NaviMesh();
    New_Convert_Map();

    return 0;
}

void Convert_NonAnim()
{
    fs::path rootDir = "../NonAnim/";

    for (auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".fbx" || entry.path().extension() == ".FBX")) {
            std::string fullPath = entry.path().string();   // 전체 경로
            std::string fileName = entry.path().filename().string(); // 파일명+확장자
            std::string stem = entry.path().stem().string(); // 확장자 제외한 파일명
            std::string ext = entry.path().extension().string(); // 확장자만

            std::cout << "Full Path: " << fullPath << "\n";
            std::cout << "File Name: " << fileName << "\n";
            std::cout << "Stem: " << stem << "\n";
            std::cout << "Extension: " << ext << "\n" << "\n";

            _uint           iFlag = { aiProcess_PreTransformVertices | aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(fullPath, iFlag);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cerr << "Error: " << importer.GetErrorString() << std::endl;
                continue;
            }

            json jModel;
            size_t iNumMeshes = {};

            jModel["Num_Meshes"] = iNumMeshes = scene->mNumMeshes;

            json jMeshes = json::array();

            for (size_t i = 0; i < iNumMeshes; ++i)
            {
                aiMesh* pMesh = scene->mMeshes[i];

                cout << "Mesh[" << i << "] Name: " << pMesh->mName.C_Str()
                    << ", Bones: " << pMesh->mNumBones << endl;

                json jMesh;

				jMesh["Mesh_Name"] = string(pMesh->mName.C_Str());

                jMesh["Material_Index"] = pMesh->mMaterialIndex;

                jMesh["Num_Vertices"] = pMesh->mNumVertices;

                jMesh["Num_Indices"] = pMesh->mNumFaces * 3;

                jMesh["Num_Faces"] = pMesh->mNumFaces;

                json jVertices = json::array();

                for (size_t j = 0; j < pMesh->mNumVertices; ++j)
                {
                    json jVertex;
                    jVertex["Position"] = { pMesh->mVertices[j].x, pMesh->mVertices[j].y, pMesh->mVertices[j].z };
                    jVertex["Normal"] = { pMesh->mNormals[j].x, pMesh->mNormals[j].y, pMesh->mNormals[j].z };

                    if (pMesh->HasTextureCoords(0))
                        jVertex["Texcoord"] = { pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y };

                    if (pMesh->HasTangentsAndBitangents()) {
                        jVertex["Tangent"] = { pMesh->mTangents[j].x, pMesh->mTangents[j].y, pMesh->mTangents[j].z };
                        jVertex["Binormal"] = { pMesh->mBitangents[j].x, pMesh->mBitangents[j].y, pMesh->mBitangents[j].z };
                    }
                    else {
                        jVertex["Tangent"] = { 0.f, 0.f, 0.f };
                        jVertex["Binormal"] = { 0.f, 0.f, 0.f };
                    }
                    jVertices.push_back(jVertex);
                }

                jMesh["Vertices"] = jVertices;

                json jFaces = json::array();
                for (size_t j = 0; j < pMesh->mNumFaces; ++j)
                {
                    json jFace;
                    jFace["Indices"] = { pMesh->mFaces[j].mIndices[0], pMesh->mFaces[j].mIndices[1], pMesh->mFaces[j].mIndices[2] };

                    jFaces.push_back(jFace);
                }
                jMesh["Faces"] = jFaces;


                jMeshes.push_back(jMesh);
            }

            jModel["Meshes"] = jMeshes;

            json jMaterials = json::array();

            for (size_t j = 0; j < scene->mNumMaterials; ++j)
            {
                aiMaterial* pMaterial = scene->mMaterials[j];

                json jMaterial = json::array();

                for (size_t k = 0; k < AI_TEXTURE_TYPE_MAX; k++)
                {
                    _uint	iNumTextures = pMaterial->GetTextureCount(static_cast<aiTextureType>(k));

                    json jTextures = json::array();

                    for (size_t ii = 0; ii < iNumTextures; ii++)
                    {
                        aiString			strTexturePath = {};

                        char			szFileName[MAX_PATH] = {};
                        char			szExt[MAX_PATH] = {};

                        if (FAILED(pMaterial->GetTexture(static_cast<aiTextureType>(k), ii, &strTexturePath)))
                            cout << "Failed to get texture path for material " << j << ", texture type " << k << ", index " << ii << endl;

                        _splitpath_s(strTexturePath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szExt, MAX_PATH);

                        char			szFileExt[MAX_PATH] = {};
                        strcpy_s(szFileExt, szFileName);
                        strcat_s(szFileExt, szExt);

                        json jTextureInfo;
                        jTextureInfo["Texture_Type"] = GetTextureTypeName(static_cast<aiTextureType>(k));
                        jTextureInfo["Texture_Name"] = szFileExt;
                        jTextures.push_back(jTextureInfo);
                    }
                    jMaterial.push_back(jTextures);
                }
                jMaterials.push_back(jMaterial);
            }

            jModel["Materials"] = jMaterials;


            string strExportPath("../Export/" + stem + ".JSON");

            ofstream file(strExportPath);
            if (!file.is_open())
                cout << "Failed to open file for writing" << endl;

            file << jModel.dump(4);
            file.close();

            cout << "\n";
        }
    }
}

void Convert_Anim()
{
    fs::path rootDir = "../Anim/";

    for (auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (entry.is_regular_file() && (entry.path().extension() == ".fbx" || entry.path().extension() == ".FBX")) {
            std::string fullPath = entry.path().string();   // 전체 경로
            std::string fileName = entry.path().filename().string(); // 파일명+확장자
            std::string stem = entry.path().stem().string(); // 확장자 제외한 파일명
            std::string ext = entry.path().extension().string(); // 확장자만

            std::cout << "Full Path: " << fullPath << "\n";
            std::cout << "File Name: " << fileName << "\n";
            std::cout << "Stem: " << stem << "\n";
            std::cout << "Extension: " << ext << "\n" << "\n";

            _uint           iFlag = { aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast };
            Assimp::Importer importer;
            const aiScene* scene = importer.ReadFile(fullPath, iFlag);

            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
                std::cerr << "Error: " << importer.GetErrorString() << std::endl;
                continue;
            }

            json jModel;
            size_t iNumMeshes = {};

            jModel["Num_Meshes"] = iNumMeshes = scene->mNumMeshes;

            json jMeshes = json::array();

            for (size_t i = 0; i < iNumMeshes; ++i)
            {
                aiMesh* pMesh = scene->mMeshes[i];

                cout << "Mesh[" << i << "] Name: " << pMesh->mName.C_Str()
                    << ", Bones: " << pMesh->mNumBones << endl;

                json jMesh;

                jMesh["Mesh_Name"] = string(pMesh->mName.C_Str());

                jMesh["Material_Index"] = pMesh->mMaterialIndex;

                jMesh["Num_Vertices"] = pMesh->mNumVertices;

                jMesh["Num_Indices"] = pMesh->mNumFaces * 3;

				jMesh["Num_Faces"] = pMesh->mNumFaces;

                json jVertices = json::array();

                for (size_t j = 0; j < pMesh->mNumVertices; ++j)
                {
                    json jVertex;
                    jVertex["Position"] = { pMesh->mVertices[j].x, pMesh->mVertices[j].y, pMesh->mVertices[j].z };
                    jVertex["Normal"] = { pMesh->mNormals[j].x, pMesh->mNormals[j].y, pMesh->mNormals[j].z };
                    if (pMesh->HasTextureCoords(0))
                        jVertex["Texcoord"] = { pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y };

                    if (pMesh->HasTangentsAndBitangents()) {
                        jVertex["Tangent"] = { pMesh->mTangents[j].x, pMesh->mTangents[j].y, pMesh->mTangents[j].z };
                        jVertex["Binormal"] = { pMesh->mBitangents[j].x, pMesh->mBitangents[j].y, pMesh->mBitangents[j].z };
                    }
                    else {
                        jVertex["Tangent"] = { 0.f, 0.f, 0.f };
                        jVertex["Binormal"] = { 0.f, 0.f, 0.f };
                    }

                    jVertices.push_back(jVertex);
                }

				json jVertex_Boneinfo = json::array();

                for (size_t j = 0; j < pMesh->mNumBones; ++j)
                {
                    aiBone* pBone = pMesh->mBones[j];
                    json jBone;
					jBone["Bone_Name"] = string(pBone->mName.C_Str());

                    json jWeights = json::array();
                    for (size_t k = 0; k < pBone->mNumWeights; ++k)
                    {
                        json jWeight;
                        jWeight["VertexId"] = pBone->mWeights[k].mVertexId;
                        jWeight["BlendWeight"] = pBone->mWeights[k].mWeight;
                        jWeights.push_back(jWeight);
                    }

                    _float4x4 OffsetMatrix = {};
					memcpy(&OffsetMatrix, &pBone->mOffsetMatrix, sizeof(_float4x4));
                    XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));

                    json jMatrix = json::array();
                    for (size_t k = 0; k < 4; ++k)
                    {
                        json iRow = { OffsetMatrix.m[k][0], OffsetMatrix.m[k][1], OffsetMatrix.m[k][2], OffsetMatrix.m[k][3] };
                        jMatrix.push_back(iRow);
                    }
					jBone["OffsetMatrix"] = jMatrix;
                    jBone["Weights"] = jWeights;
					jVertex_Boneinfo.push_back(jBone);
				}
                jMesh["Bones"] = jVertex_Boneinfo;


                

                jMesh["Vertices"] = jVertices;

                json jFaces = json::array();
                for (size_t j = 0; j < pMesh->mNumFaces; ++j)
                {
                    json jFace;
                    jFace["Indices"] = { pMesh->mFaces[j].mIndices[0], pMesh->mFaces[j].mIndices[1], pMesh->mFaces[j].mIndices[2] };

                    jFaces.push_back(jFace);
                }
                jMesh["Faces"] = jFaces;


                jMeshes.push_back(jMesh);
            }

            jModel["Meshes"] = jMeshes;

            json jMaterials = json::array();

            for (size_t j = 0; j < scene->mNumMaterials; ++j)
            {
                aiMaterial* pMaterial = scene->mMaterials[j];

                json jMaterial = json::array();

                for (size_t k = 0; k < AI_TEXTURE_TYPE_MAX; k++)
                {
                    _uint	iNumTextures = pMaterial->GetTextureCount(static_cast<aiTextureType>(k));

                    json jTextures = json::array();

                    for (size_t ii = 0; ii < iNumTextures; ii++)
                    {
                        aiString			strTexturePath = {};

                        char			szFileName[MAX_PATH] = {};
                        char			szExt[MAX_PATH] = {};

                        if (FAILED(pMaterial->GetTexture(static_cast<aiTextureType>(k), ii, &strTexturePath)))
                            cout << "Failed to get texture path for material " << j << ", texture type " << k << ", index " << ii << endl;

                        _splitpath_s(strTexturePath.data, nullptr, 0, nullptr, 0, szFileName, MAX_PATH, szExt, MAX_PATH);

                        char			szFileExt[MAX_PATH] = {};
                        strcpy_s(szFileExt, szFileName);
                        strcat_s(szFileExt, szExt);

                        json jTextureInfo;
                        jTextureInfo["Texture_Type"] = GetTextureTypeName(static_cast<aiTextureType>(k));
                        jTextureInfo["Texture_Name"] = szFileExt;
                        jTextures.push_back(jTextureInfo);
                    }
                    jMaterial.push_back(jTextures);
                }
                jMaterials.push_back(jMaterial);
            }

            jModel["Materials"] = jMaterials;

            json jNode;
			aiNode* pRootNode = scene->mRootNode;

            Ready_Bones(pRootNode, jNode);

			jModel["RootNode"] = jNode;


            string strExportPath("../Export/" + stem + ".JSON");

            ofstream file(strExportPath);
            if (!file.is_open())
                cout << "Failed to open file for writing" << endl;

            file << jModel.dump(4);
            file.close();

            cout << "\n";
        }
    }
}

const char* GetTextureTypeName(aiTextureType eType)
{
    switch (eType)
    {
    case aiTextureType_NONE:                return "None";
    case aiTextureType_DIFFUSE:             return "Diffuse";
    case aiTextureType_SPECULAR:            return "Specular";
    case aiTextureType_AMBIENT:             return "Ambient";
    case aiTextureType_EMISSIVE:            return "Emissive";
    case aiTextureType_HEIGHT:              return "Height";
    case aiTextureType_NORMALS:             return "Normals";
    case aiTextureType_SHININESS:           return "Shininess";
    case aiTextureType_OPACITY:             return "Opacity";
    case aiTextureType_DISPLACEMENT:        return "Displacement";
    case aiTextureType_LIGHTMAP:            return "Lightmap";
    case aiTextureType_REFLECTION:          return "Reflection";
    case aiTextureType_BASE_COLOR:          return "BaseColor";
    case aiTextureType_NORMAL_CAMERA:       return "NormalCamera";
    case aiTextureType_EMISSION_COLOR:      return "EmissionColor";
    case aiTextureType_METALNESS:           return "Metalness";
    case aiTextureType_DIFFUSE_ROUGHNESS:   return "Roughness";
    case aiTextureType_AMBIENT_OCCLUSION:   return "AO";
    case aiTextureType_UNKNOWN:             return "Unknown";

        // 추가된 glTF/PBR 관련 타입들
    case aiTextureType_SHEEN:                   return "Sheen";
    case aiTextureType_CLEARCOAT:               return "Clearcoat";
    case aiTextureType_TRANSMISSION:            return "Transmission";
    case aiTextureType_MAYA_BASE:               return "MayaBase";
    case aiTextureType_MAYA_SPECULAR:           return "MayaSpecular";
    case aiTextureType_MAYA_SPECULAR_COLOR:     return "MayaSpecularColor";
    case aiTextureType_MAYA_SPECULAR_ROUGHNESS: return "MayaSpecularRoughness";
    case aiTextureType_ANISOTROPY:              return "Anisotropy";
    case aiTextureType_GLTF_METALLIC_ROUGHNESS: return "GLTF_MetallicRoughness";


    default:                                return "Unknown";
    }
}

void Ready_Bones(const aiNode* pAINode, json& jNode)
{
	jNode["Node_Name"] = string(pAINode->mName.C_Str());

    _float4x4 Transformation;
    memcpy(&Transformation, &pAINode->mTransformation, sizeof(_float4x4));
	// row-major -> column-major
    XMStoreFloat4x4(&Transformation, XMMatrixTranspose(XMLoadFloat4x4(&Transformation)));

	json jMatrix = json::array();
    for (size_t i = 0; i < 4; i++)
    {
        json iRow = { Transformation.m[i][0], Transformation.m[i][1], Transformation.m[i][2], Transformation.m[i][3] };
		jMatrix.push_back(iRow);
    }

	jNode["Transformation"] = jMatrix;
	jNode["Num_Children"] = pAINode->mNumChildren;

	json jChildren = json::array();
    for (size_t i = 0; i < pAINode->mNumChildren; ++i)
    {
        json jChild;
		Ready_Bones(pAINode->mChildren[i], jChild);
		jChildren.push_back(jChild);
    }
	jNode["Children"] = jChildren;
}

void New_Convert_NoAnim()
{
    fs::path rootDir = "../NonAnim/";

    for (auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension();
        if (ext != ".fbx" && ext != ".FBX" && ext != ".obj" && ext != ".OBJ") continue;

        string fullPath = entry.path().string();
        string stem = entry.path().stem().string();

        _uint iFlag = aiProcess_PreTransformVertices | aiProcess_ConvertToLeftHanded |
            aiProcessPreset_TargetRealtime_Fast;
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(fullPath, iFlag);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cerr << importer.GetErrorString() << endl;
            continue;
        }

        ofstream file("../Export/" + stem + ".ysh", ios::binary);

        // Header
        WriteVal(file, YSH_MAGIC);
        WriteVal(file, YSH_VERSION);
        WriteVal(file, (uint32_t)0); // NONANIM

        // Meshes
        WriteVal(file, (uint32_t)scene->mNumMeshes);
        for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
        {
            aiMesh* pMesh = scene->mMeshes[i];
            WriteStr(file, string(pMesh->mName.C_Str()));
            WriteVal(file, (uint32_t)pMesh->mMaterialIndex);
            WriteVal(file, (uint32_t)pMesh->mNumVertices);

            // 버텍스 배열 구성 후 통째로 write
            vector<VTXMESH_DATA> vertices(pMesh->mNumVertices);
            for (uint32_t j = 0; j < pMesh->mNumVertices; ++j)
            {
                VTXMESH_DATA& v = vertices[j];
                v.vPosition = { pMesh->mVertices[j].x, pMesh->mVertices[j].y, pMesh->mVertices[j].z };
                v.vNormal = { pMesh->mNormals[j].x,  pMesh->mNormals[j].y,  pMesh->mNormals[j].z };
                v.vTexcoord = pMesh->HasTextureCoords(0)
                    ? XMFLOAT2{ pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y }
                : XMFLOAT2{ 0.f, 0.f };
                if (pMesh->HasTangentsAndBitangents()) {
                    v.vTangent = { pMesh->mTangents[j].x,    pMesh->mTangents[j].y,    pMesh->mTangents[j].z };
                    v.vBinormal = { pMesh->mBitangents[j].x,  pMesh->mBitangents[j].y, pMesh->mBitangents[j].z };
                }
            }
            file.write(reinterpret_cast<const char*>(vertices.data()), sizeof(VTXMESH_DATA) *
                vertices.size());


            // 인덱스 통째로 write
            uint32_t numIndices = pMesh->mNumFaces * 3;
            WriteVal(file, numIndices);
            for (uint32_t j = 0; j < pMesh->mNumFaces; ++j) {
                WriteVal(file, (uint32_t)pMesh->mFaces[j].mIndices[0]);
                WriteVal(file, (uint32_t)pMesh->mFaces[j].mIndices[1]);
                WriteVal(file, (uint32_t)pMesh->mFaces[j].mIndices[2]);
            }

            cout << "  Mesh[" << i << "] " << pMesh->mName.C_Str() << " ... done" << endl;
        }


        // Materials
        json jMatInfo;                                                  // [추가]
        jMatInfo["model"] = stem;                                       // [추가]
        jMatInfo["material_count"] = (uint32_t)scene->mNumMaterials;     // [추가]
        json jMaterialArray = json::array();                            // [추가]

        WriteVal(file, (uint32_t)scene->mNumMaterials);
        for (uint32_t j = 0; j < scene->mNumMaterials; ++j)
        {
            aiMaterial* pMaterial = scene->mMaterials[j];

            json jMat;                                                  // [추가]
            jMat["index"] = j;                                          // [추가]
            json jTexArray = json::array();                             // [추가]

            for (uint32_t k = 0; k < AI_TEXTURE_TYPE_MAX; ++k)
            {
                uint32_t iNumTex = pMaterial->GetTextureCount(static_cast<aiTextureType>(k));
                WriteVal(file, iNumTex);
                for (uint32_t ii = 0; ii < iNumTex; ++ii)
                {
                    aiString strPath;
                    pMaterial->GetTexture(static_cast<aiTextureType>(k), ii, &strPath);
                    char szName[MAX_PATH] = {}, szExt[MAX_PATH] = {};
                    _splitpath_s(strPath.data, nullptr, 0, nullptr, 0, szName, MAX_PATH, szExt, MAX_PATH);
                    string texName = string(szName) + string(szExt);
                    WriteStr(file, texName);

                    json jTex;                                                          // [추가]
                    jTex["type"] = GetTextureTypeName(static_cast<aiTextureType>(k));    // [추가]
                    jTex["slot"] = ii;                                                  // [추가]
                    jTex["name"] = texName;                                             // [추가]
                    jTexArray.push_back(jTex);                                          // [추가]
                }
            }
            jMat["textures"] = jTexArray;                               // [추가]
            jMaterialArray.push_back(jMat);                            // [추가]
        }

        jMatInfo["materials"] = jMaterialArray;                                 // [추가]
        ofstream matFile("../Export/" + stem + "_material.json");               // [추가]
        matFile << jMatInfo.dump(4);                                            // [추가]
        matFile.close();                                                        // [추가]
        cout << "Material exported: " << stem << "_material.json\n";            // [추가]

        cout << "Exported: " << stem << ".ysh\n";
    }
}

void New_Convert_Anim()
{
    fs::path rootDir = "../Anim/";

    for (auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension();
        if (ext != ".fbx" && ext != ".FBX") continue;

        string fullPath = entry.path().string();
        string stem = entry.path().stem().string();

        _uint iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(fullPath, iFlag);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cerr << importer.GetErrorString() << endl;
            continue;
        }

        ofstream file("../Export/" + stem + ".ysh", ios::binary);

        // Header
        WriteVal(file, YSH_MAGIC);
        WriteVal(file, YSH_VERSION);
        WriteVal(file, (uint32_t)1); // ANIM

        // BoneHierarchy 먼저
        New_Ready_Bones(file, scene->mRootNode);

        //PrintParentBone(scene->mRootNode, "Weapone_Special_6");

        // Meshes
        WriteVal(file, (uint32_t)scene->mNumMeshes);
        for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
        {
            aiMesh* pMesh = scene->mMeshes[i];
            WriteStr(file, string(pMesh->mName.C_Str()));
            WriteVal(file, (uint32_t)pMesh->mMaterialIndex);
            WriteVal(file, (uint32_t)pMesh->mNumVertices);

            // 버텍스 배열 구성
            vector<VTXANIMMESH_DATA> vertices(pMesh->mNumVertices);
            for (uint32_t j = 0; j < pMesh->mNumVertices; ++j)
            {
                VTXANIMMESH_DATA& v = vertices[j];
                v.vPosition = { pMesh->mVertices[j].x, pMesh->mVertices[j].y, pMesh->mVertices[j].z };
                v.vNormal = { pMesh->mNormals[j].x,  pMesh->mNormals[j].y,  pMesh->mNormals[j].z };
                v.vTexcoord = pMesh->HasTextureCoords(0)
                    ? XMFLOAT2{ pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y }
                : XMFLOAT2{ 0.f, 0.f };
                if (pMesh->HasTangentsAndBitangents()) {
                    v.vTangent = { pMesh->mTangents[j].x,   pMesh->mTangents[j].y,   pMesh->mTangents[j].z
                    };
                    v.vBinormal = { pMesh->mBitangents[j].x, pMesh->mBitangents[j].y, pMesh->mBitangents[j].z
                    };
                }
                v.vBlendIndex = { 0, 0, 0, 0 };
                v.vBlendWeight = { 0.f, 0.f, 0.f, 0.f };
            }

            // 블렌드 가중치 사전 산포 (bone → vertex)
            vector<uint32_t> slotCount(pMesh->mNumVertices, 0);
            for (uint32_t j = 0; j < pMesh->mNumBones; ++j)
            {
                aiBone* pBone = pMesh->mBones[j];

                for (uint32_t k = 0; k < pBone->mNumWeights; ++k)
                {
                    uint32_t vId = pBone->mWeights[k].mVertexId;
                    uint32_t slot = slotCount[vId];
                    if (slot >= 4) continue; // 최대 4본
                    reinterpret_cast<uint32_t*>(&vertices[vId].vBlendIndex)[slot] = j;
                    reinterpret_cast<float*>(&vertices[vId].vBlendWeight)[slot] =
                        pBone->mWeights[k].mWeight;
                    slotCount[vId]++;
                }
            }
            file.write(reinterpret_cast<const char*>(vertices.data()), sizeof(VTXANIMMESH_DATA) *
                vertices.size());

            // 인덱스
            uint32_t numIndices = pMesh->mNumFaces * 3;
            WriteVal(file, numIndices);
            for (uint32_t j = 0; j < pMesh->mNumFaces; ++j) {
                WriteVal(file, (uint32_t)pMesh->mFaces[j].mIndices[0]);
                WriteVal(file, (uint32_t)pMesh->mFaces[j].mIndices[1]);
                WriteVal(file, (uint32_t)pMesh->mFaces[j].mIndices[2]);
            }

            // 메시별 본 (이름 + OffsetMatrix만)
            WriteVal(file, (uint32_t)pMesh->mNumBones);
            for (uint32_t j = 0; j < pMesh->mNumBones; ++j)
            {
                aiBone* pBone = pMesh->mBones[j];
                WriteStr(file, string(pBone->mName.C_Str()));

                _float4x4 OffsetMatrix;
                memcpy(&OffsetMatrix, &pBone->mOffsetMatrix, sizeof(_float4x4));
                XMStoreFloat4x4(&OffsetMatrix, XMMatrixTranspose(XMLoadFloat4x4(&OffsetMatrix)));
                WriteVal(file, OffsetMatrix);
            }

            cout << "  Mesh[" << i << "] " << pMesh->mName.C_Str() << " ... done" << endl;
        }

        // 임시 중단코드
        //return;

        // Materials
        json jMatInfo;                                                  // [추가]
        jMatInfo["model"] = stem;                                       // [추가]
        jMatInfo["material_count"] = (uint32_t)scene->mNumMaterials;     // [추가]
        json jMaterialArray = json::array();                            // [추가]

        WriteVal(file, (uint32_t)scene->mNumMaterials);
        for (uint32_t j = 0; j < scene->mNumMaterials; ++j)
        {
            aiMaterial* pMaterial = scene->mMaterials[j];

            json jMat;                                                  // [추가]
            jMat["index"] = j;                                          // [추가]
            json jTexArray = json::array();                             // [추가]

            for (uint32_t k = 0; k < AI_TEXTURE_TYPE_MAX; ++k)
            {
                uint32_t iNumTex = pMaterial->GetTextureCount(static_cast<aiTextureType>(k));
                WriteVal(file, iNumTex);
                for (uint32_t ii = 0; ii < iNumTex; ++ii)
                {
                    aiString strPath;
                    pMaterial->GetTexture(static_cast<aiTextureType>(k), ii, &strPath);
                    char szName[MAX_PATH] = {}, szExt[MAX_PATH] = {};
                    _splitpath_s(strPath.data, nullptr, 0, nullptr, 0, szName, MAX_PATH, szExt, MAX_PATH);
                    string texName = string(szName) + string(szExt);
                    WriteStr(file, texName);

                    json jTex;                                                          // [추가]
                    jTex["type"] = GetTextureTypeName(static_cast<aiTextureType>(k));    // [추가]
                    jTex["slot"] = ii;                                                  // [추가]
                    jTex["name"] = texName;                                             // [추가]
                    jTexArray.push_back(jTex);                                          // [추가]
                }
            }
            jMat["textures"] = jTexArray;                               // [추가]
            jMaterialArray.push_back(jMat);                            // [추가]
        }

        WriteVal(file, (uint32_t)scene->mNumAnimations);
        for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
        {
            aiAnimation* pAnim = scene->mAnimations[i];

            WriteStr(file, string(pAnim->mName.C_Str()));
            WriteVal(file, (float)pAnim->mDuration);
            WriteVal(file, (float)(pAnim->mTicksPerSecond != 0.0 ? pAnim->mTicksPerSecond : 25.0));
            WriteVal(file, (uint32_t)pAnim->mNumChannels);

            cout << "  Anim[" << i << "] " << pAnim->mName.C_Str()
                << " channels: " << pAnim->mNumChannels << endl;

            for (uint32_t j = 0; j < pAnim->mNumChannels; ++j)
            {
                aiNodeAnim* pChannel = pAnim->mChannels[j];

                WriteStr(file, string(pChannel->mNodeName.C_Str()));

                // Position / Rotation / Scaling 키 개수가 다를 수 있어서 max 기준
                uint32_t numKeyFrames = pChannel->mNumPositionKeys;
                if (pChannel->mNumRotationKeys > numKeyFrames) numKeyFrames = pChannel->mNumRotationKeys;
                if (pChannel->mNumScalingKeys > numKeyFrames) numKeyFrames = pChannel->mNumScalingKeys;

                WriteVal(file, numKeyFrames);

                for (uint32_t k = 0; k < numKeyFrames; ++k)
                {
                    KEYFRAME_DATA kf = {};

                    // 각각 범위 초과시 마지막 키로 클램프
                    uint32_t sIdx = min(k, pChannel->mNumScalingKeys - 1);
                    uint32_t rIdx = min(k, pChannel->mNumRotationKeys - 1);
                    uint32_t tIdx = min(k, pChannel->mNumPositionKeys - 1);

                    kf.vScale = {
                        (float)pChannel->mScalingKeys[sIdx].mValue.x,
                        (float)pChannel->mScalingKeys[sIdx].mValue.y,
                        (float)pChannel->mScalingKeys[sIdx].mValue.z
                    };

                    kf.vRotation = {
                        (float)pChannel->mRotationKeys[rIdx].mValue.x,
                        (float)pChannel->mRotationKeys[rIdx].mValue.y,
                        (float)pChannel->mRotationKeys[rIdx].mValue.z,
                        (float)pChannel->mRotationKeys[rIdx].mValue.w
                    };

                    kf.vTranslation = {
                        (float)pChannel->mPositionKeys[tIdx].mValue.x,
                        (float)pChannel->mPositionKeys[tIdx].mValue.y,
                        (float)pChannel->mPositionKeys[tIdx].mValue.z
                    };

                    // TrackPosition은 키가 가장 많은 쪽의 시간 기준
                    if (pChannel->mNumPositionKeys >= pChannel->mNumRotationKeys &&
                        pChannel->mNumPositionKeys >= pChannel->mNumScalingKeys)
                        kf.fTrackPosition = (float)pChannel->mPositionKeys[tIdx].mTime;
                    else if (pChannel->mNumRotationKeys >= pChannel->mNumScalingKeys)
                        kf.fTrackPosition = (float)pChannel->mRotationKeys[rIdx].mTime;
                    else
                        kf.fTrackPosition = (float)pChannel->mScalingKeys[sIdx].mTime;

                    WriteVal(file, kf);
                }
            }
        }
        
        json jBoneList;
        json jBones = json::array();
        Collect_BoneList(scene->mRootNode, -1, jBones);
        jBoneList["model"] = stem;
        jBoneList["count"] = (uint32_t)jBones.size();
        jBoneList["bones"] = jBones;

        ofstream jBoneFile("../Export/" + stem + "_BoneList.json");
        jBoneFile << jBoneList.dump(4);
        jBoneFile.close();
        cout << "BoneList exported: " << stem << "_BoneList.json\n";

        json jAnimList;
        jAnimList["count"] = scene->mNumAnimations;
        json jAnims = json::array();
        for (uint32_t i = 0; i < scene->mNumAnimations; ++i)
        {
            json jAnim;
            jAnim["index"] = i;
            jAnim["name"] = string(scene->mAnimations[i]->mName.C_Str());
            jAnims.push_back(jAnim);
        }
        jAnimList["animations"] = jAnims;

        ofstream jFile("../Export/" + stem + "_AnimList.json");
        jFile << jAnimList.dump(4);
        jFile.close();
        cout << "AnimList exported: " << stem << "_AnimList.json\n";

        jMatInfo["materials"] = jMaterialArray;
        ofstream matFile("../Export/" + stem + "_material.json");
        matFile << jMatInfo.dump(4);
        matFile.close();
        cout << "Material exported: " << stem << "_material.json\n";

        cout << "Exported: " << stem << ".ysh\n";
    }
}

void New_Ready_Bones(ofstream& f, const aiNode* pAINode)
{
    WriteStr(f, string(pAINode->mName.C_Str()));

    _float4x4 Transformation;
    memcpy(&Transformation, &pAINode->mTransformation, sizeof(_float4x4));
    XMStoreFloat4x4(&Transformation, XMMatrixTranspose(XMLoadFloat4x4(&Transformation)));
    WriteVal(f, Transformation);

    WriteVal(f, (uint32_t)pAINode->mNumChildren);
    for (uint32_t i = 0; i < pAINode->mNumChildren; ++i)
        New_Ready_Bones(f, pAINode->mChildren[i]);
}

void PrintParentBone(aiNode* node, const std::string& targetBoneName) {
    if (targetBoneName == node->mName.C_Str()) {
        if (node->mParent) {
            std::cout << "Bone " << targetBoneName
                << " has parent: " << node->mParent->mName.C_Str() << std::endl;
        }
        else {
            std::cout << "Bone " << targetBoneName << " has no parent (root)." << std::endl;
        }
    }
    for (unsigned int i = 0; i < node->mNumChildren; ++i) {
        PrintParentBone(node->mChildren[i], targetBoneName);
    }
}

void Convert_NaviMesh()
{
    fs::path rootDir = "../NaviMesh/";

    for (auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension();
        if (ext != ".fbx" && ext != ".FBX") continue;

        string fullPath = entry.path().string();
        string stem = entry.path().stem().string();

        _uint iFlag = aiProcess_PreTransformVertices | aiProcess_ConvertToLeftHanded |
            aiProcessPreset_TargetRealtime_Fast;
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(fullPath, iFlag);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cerr << "NaviMesh import error: " << importer.GetErrorString() << endl;
            continue;
        }

        // 0.001 단위 양자화 — 미세 부동소수점 오차 흡수
        auto quantize = [](float f) -> int { return (int)roundf(f * 1000.f); };

        map<tuple<int, int, int>, uint32_t> vertexMap;
        vector<XMFLOAT3>           vertices;
        vector<array<uint32_t, 3>> cells;

        auto getOrAdd = [&](const XMFLOAT3& v) -> uint32_t {
            auto key = make_tuple(quantize(v.x), quantize(v.y), quantize(v.z));
            auto it = vertexMap.find(key);
            if (it != vertexMap.end()) return it->second;
            uint32_t idx = (uint32_t)vertices.size();
            vertices.push_back(v);
            vertexMap[key] = idx;
            return idx;
            };

        uint32_t flippedCount = 0;

        for (uint32_t i = 0; i < scene->mNumMeshes; ++i)
        {
            aiMesh* pMesh = scene->mMeshes[i];

            for (uint32_t j = 0; j < pMesh->mNumFaces; ++j)
            {
                const aiFace& face = pMesh->mFaces[j];
                XMFLOAT3 v0 = { pMesh->mVertices[face.mIndices[0]].x, pMesh->mVertices[face.mIndices[0]].y, pMesh->mVertices[face.mIndices[0]].z };
                XMFLOAT3 v1 = { pMesh->mVertices[face.mIndices[1]].x, pMesh->mVertices[face.mIndices[1]].y, pMesh->mVertices[face.mIndices[1]].z };
                XMFLOAT3 v2 = { pMesh->mVertices[face.mIndices[2]].x, pMesh->mVertices[face.mIndices[2]].y, pMesh->mVertices[face.mIndices[2]].z };

                float normalY = (v1.z - v0.z) * (v2.x - v0.x) - (v1.x - v0.x) * (v2.z - v0.z);
                uint32_t i0 = getOrAdd(v0);
                if (normalY < 0.f) {
                    cells.push_back({ i0, getOrAdd(v2), getOrAdd(v1) });
                    ++flippedCount;
                }
                else {
                    cells.push_back({ i0, getOrAdd(v1), getOrAdd(v2) });
                }
            }
        }

        // 파일 출력
        ofstream file("../Export/" + stem + ".nav", ios::binary);
        WriteVal(file, (uint32_t)vertices.size());
        for (auto& v : vertices)
            WriteVal(file, v);
        WriteVal(file, (uint32_t)cells.size());
        for (auto& c : cells) {
            WriteVal(file, c[0]);
            WriteVal(file, c[1]);
            WriteVal(file, c[2]);
        }

        cout << "NaviMesh Exported: " << stem << ".nav"
            << "  verts=" << vertices.size()
            << "  cells=" << cells.size()
            << "  cw_flipped=" << flippedCount << "\n";
    }
}

struct MeshEntry {
    uint32_t    meshIndex;      // scene->mMeshes 인덱스
    aiMatrix4x4 worldTransform; // 누적 월드 변환
};

static void TraverseNode(
    const aiNode* pNode,
    const aiMatrix4x4& parentTransform,
    vector<MeshEntry>& outEntries)
{
    aiMatrix4x4 world = parentTransform * pNode->mTransformation;

    for (uint32_t i = 0; i < pNode->mNumMeshes; ++i)
        outEntries.push_back({ pNode->mMeshes[i], world });

    for (uint32_t i = 0; i < pNode->mNumChildren; ++i)
        TraverseNode(pNode->mChildren[i], world, outEntries);
}

void New_Convert_Map()
{
    fs::path rootDir = "../Map/";

    if (!fs::exists(rootDir)) {
        cout << "[Map] directory not found: " << rootDir << "\n";
        return;
    }

    for (auto& entry : fs::recursive_directory_iterator(rootDir)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension();
        if (ext != ".fbx" && ext != ".FBX") continue;

        string fullPath = entry.path().string();
        string stem = entry.path().stem().string();

        // PreTransformVertices 제거 — 노드 계층 유지해서 메쉬 분리 보존
        _uint iFlag = aiProcess_ConvertToLeftHanded | aiProcessPreset_TargetRealtime_Fast;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(fullPath, iFlag);
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            cerr << "[Map] import error: " << importer.GetErrorString() << endl;
            continue;
        }

        // 노드 DFS로 메쉬 엔트리 수집 (인스턴싱 시 같은 메쉬가 다중 엔트리로 들어갈 수 있음 — 의도된 동작)
        vector<MeshEntry> entries;
        TraverseNode(scene->mRootNode, aiMatrix4x4(), entries);

        ofstream file("../Export/" + stem + ".ysh", ios::binary);

        // Header
        WriteVal(file, YSH_MAGIC);
        WriteVal(file, YSH_VERSION);
        WriteVal(file, (uint32_t)0); // NONANIM

        // _meshinfo.json 수집 준비
        json jMeshInfo;
        jMeshInfo["model"] = stem;
        jMeshInfo["mesh_count"] = (uint32_t)entries.size();
        json jMeshArray = json::array();

        // Meshes (entries 순서 기준)
        WriteVal(file, (uint32_t)entries.size());
        for (uint32_t serializedIndex = 0; serializedIndex < entries.size(); ++serializedIndex)
        {
            const MeshEntry& me = entries[serializedIndex];
            aiMesh* pMesh = scene->mMeshes[me.meshIndex];

            WriteStr(file, string(pMesh->mName.C_Str()));
            WriteVal(file, (uint32_t)pMesh->mMaterialIndex);
            WriteVal(file, (uint32_t)pMesh->mNumVertices);

            // 노말 변환용 3x3 (NonUniform Scale 가정 없음 — 필요 시 Inverse+Transpose로 교체)
            aiMatrix3x3 normalMat(me.worldTransform);

            vector<VTXMESH_DATA> vertices(pMesh->mNumVertices);
            for (uint32_t j = 0; j < pMesh->mNumVertices; ++j)
            {
                VTXMESH_DATA& v = vertices[j];

                // Position — 평행이동 포함 (w=1)
                aiVector3D pos = pMesh->mVertices[j];
                pos = me.worldTransform * pos;
                v.vPosition = { pos.x, pos.y, pos.z };

                // Normal — 회전만 (w=0), 정규화
                aiVector3D nor = normalMat * pMesh->mNormals[j];
                nor.Normalize();
                v.vNormal = { nor.x, nor.y, nor.z };

                // Texcoord
                v.vTexcoord = pMesh->HasTextureCoords(0)
                    ? XMFLOAT2{ pMesh->mTextureCoords[0][j].x, pMesh->mTextureCoords[0][j].y }
                : XMFLOAT2{ 0.f, 0.f };

                // Tangent / Binormal — 회전만, 정규화
                if (pMesh->HasTangentsAndBitangents()) {
                    aiVector3D tan = normalMat * pMesh->mTangents[j];   tan.Normalize();
                    aiVector3D bin = normalMat * pMesh->mBitangents[j]; bin.Normalize();
                    v.vTangent = { tan.x, tan.y, tan.z };
                    v.vBinormal = { bin.x, bin.y, bin.z };
                }
            }
            file.write(reinterpret_cast<const char*>(vertices.data()),
                sizeof(VTXMESH_DATA) * vertices.size());

            // 인덱스
            uint32_t numIndices = pMesh->mNumFaces * 3;
            WriteVal(file, numIndices);
            for (uint32_t j = 0; j < pMesh->mNumFaces; ++j) {
                WriteVal(file, (uint32_t)pMesh->mFaces[j].mIndices[0]);
                WriteVal(file, (uint32_t)pMesh->mFaces[j].mIndices[1]);
                WriteVal(file, (uint32_t)pMesh->mFaces[j].mIndices[2]);
            }

            // _meshinfo.json 엔트리 추가
            json jMesh;
            jMesh["index"] = serializedIndex;
            jMesh["name"] = string(pMesh->mName.C_Str());
            jMesh["material_index"] = pMesh->mMaterialIndex;
            jMesh["vertex_count"] = pMesh->mNumVertices;
            jMesh["face_count"] = pMesh->mNumFaces;
            jMeshArray.push_back(jMesh);

            cout << "  Mesh[" << serializedIndex << "] " << pMesh->mName.C_Str() << " ... done" << endl;
        }

        // _meshinfo.json — materials 컨테이너
        json jMaterialArray = json::array();

        // Materials
        WriteVal(file, (uint32_t)scene->mNumMaterials);
        for (uint32_t j = 0; j < scene->mNumMaterials; ++j)
        {
            aiMaterial* pMaterial = scene->mMaterials[j];

            json jMat;
            jMat["index"] = j;
            json jTexArray = json::array();

            for (uint32_t k = 0; k < AI_TEXTURE_TYPE_MAX; ++k)
            {
                uint32_t iNumTex = pMaterial->GetTextureCount(static_cast<aiTextureType>(k));
                WriteVal(file, iNumTex);
                for (uint32_t ii = 0; ii < iNumTex; ++ii)
                {
                    aiString strPath;
                    pMaterial->GetTexture(static_cast<aiTextureType>(k), ii, &strPath);
                    char szName[MAX_PATH] = {}, szExt[MAX_PATH] = {};
                    _splitpath_s(strPath.data, nullptr, 0, nullptr, 0, szName, MAX_PATH, szExt, MAX_PATH);
                    string texName = string(szName) + string(szExt);
                    WriteStr(file, texName);

                    // JSON에도 같이 기록
                    json jTex;
                    jTex["type"] = GetTextureTypeName(static_cast<aiTextureType>(k));
                    jTex["slot"] = ii;            // 같은 타입에 여러 장일 때 구분용. 필요 없으면 제거
                    jTex["name"] = texName;
                    jTexArray.push_back(jTex);
                }
            }
            jMat["textures"] = jTexArray;
            jMaterialArray.push_back(jMat);
        }

        // _meshinfo.json 기록 (기존 부분에 두 줄 추가)
        jMeshInfo["meshes"] = jMeshArray;
        jMeshInfo["material_count"] = (uint32_t)scene->mNumMaterials;   // 추가
        jMeshInfo["materials"] = jMaterialArray;                   // 추가
        ofstream jsonFile("../Export/" + stem + "_meshinfo.json");
        jsonFile << jMeshInfo.dump(2);
        jsonFile.close();



        cout << "Exported: " << stem << ".ysh (" << entries.size() << " meshes)\n";
        cout << "MeshInfo: " << stem << "_meshinfo.json\n";
    }
}

void Collect_BoneList(const aiNode* pAINode, _int parentIdx, json& jBoneArray)
{
    // CDataLoader::Read_BoneHierarchy와 동일한 DFS 순서를 유지해야
    // 런타임 m_Bones[i]와 JSON index가 1:1로 일치한다.
    _int myIdx = (_int)jBoneArray.size();

    json jBone;
    jBone["index"] = myIdx;
    jBone["name"] = string(pAINode->mName.C_Str());
    jBone["parent_index"] = parentIdx;

    _float4x4 Transformation;
    memcpy(&Transformation, &pAINode->mTransformation, sizeof(_float4x4));
    XMStoreFloat4x4(&Transformation, XMMatrixTranspose(XMLoadFloat4x4(&Transformation)));

    json jMatrix = json::array();
    for (size_t i = 0; i < 4; ++i)
    {
        json iRow = { Transformation.m[i][0], Transformation.m[i][1],
                      Transformation.m[i][2], Transformation.m[i][3] };
        jMatrix.push_back(iRow);
    }
    jBone["transformation"] = jMatrix;

    jBoneArray.push_back(jBone);

    for (uint32_t i = 0; i < pAINode->mNumChildren; ++i)
        Collect_BoneList(pAINode->mChildren[i], myIdx, jBoneArray);
}

