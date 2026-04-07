#include "Model3D.h"
#include <chrono>
#include <cstdint>
#include <fstream>
#include <unordered_map>

namespace {
constexpr uint32_t kModelCacheMagic = 0x48564D57; // WMVH
constexpr uint32_t kModelCacheVersion = 1;

struct ModelCacheEntry {
	std::vector<MeshComponent> meshes;
	std::vector<std::string> textureFileNames;
};

std::unordered_map<std::string, ModelCacheEntry> g_modelCache;

bool GetFileWriteTime(const std::string& path, ULONGLONG& outWriteTime) {
	WIN32_FILE_ATTRIBUTE_DATA attributes{};
	if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attributes)) {
		return false;
	}

	ULARGE_INTEGER fileTime{};
	fileTime.LowPart = attributes.ftLastWriteTime.dwLowDateTime;
	fileTime.HighPart = attributes.ftLastWriteTime.dwHighDateTime;
	outWriteTime = fileTime.QuadPart;
	return true;
}

bool WriteString(std::ofstream& stream, const std::string& value) {
	const uint32_t length = static_cast<uint32_t>(value.size());
	stream.write(reinterpret_cast<const char*>(&length), sizeof(length));
	if (length > 0) {
		stream.write(value.data(), length);
	}
	return stream.good();
}

bool ReadString(std::ifstream& stream, std::string& value) {
	uint32_t length = 0;
	stream.read(reinterpret_cast<char*>(&length), sizeof(length));
	if (!stream.good()) {
		return false;
	}

	value.resize(length);
	if (length > 0) {
		stream.read(&value[0], length);
	}
	return stream.good();
}
}

Model3D::~Model3D() {
	unload();
}

bool 
Model3D::load(const std::string& path) {
	SetPath(path);
	SetState(ResourceState::Loading);

	auto cacheIt = g_modelCache.find(path);
	if (cacheIt != g_modelCache.end()) {
		m_meshes = cacheIt->second.meshes;
		textureFileNames = cacheIt->second.textureFileNames;
		SetState(ResourceState::Loaded);
		return true;
	}

	const bool success = init();
	SetState(success ? ResourceState::Loaded : ResourceState::Failed);
	return success;
}

bool Model3D::init()
{
	if (m_modelType != ModelType::FBX) {
		return true;
	}

	m_meshes.clear();
	textureFileNames.clear();

	const std::string cachePath = GetBinaryCachePath();
	if (IsBinaryCacheUpToDate(m_filePath, cachePath) && LoadBinaryCache(cachePath)) {
		g_modelCache[m_filePath] = ModelCacheEntry{ m_meshes, textureFileNames };
		return true;
	}

	const auto begin = std::chrono::high_resolution_clock::now();
	const std::vector<MeshComponent> loadedMeshes = LoadFBXModel(m_filePath);
	const auto end = std::chrono::high_resolution_clock::now();
	const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();

	if (loadedMeshes.empty()) {
		return false;
	}

	m_meshes = loadedMeshes;
	g_modelCache[m_filePath] = ModelCacheEntry{ m_meshes, textureFileNames };
	SaveBinaryCache(cachePath);

	const std::wstring modelPathW(m_filePath.begin(), m_filePath.end());
	MESSAGE("ModelLoader", "ModelLoader",
		L"Loaded model '" << modelPathW << L"' in " << elapsedMs << L" ms. Meshes: " << m_meshes.size())
	return true;
}

void Model3D::unload()
{
	if (lScene) {
		lScene->Destroy();
		lScene = nullptr;
	}
	if (lSdkManager) {
		lSdkManager->Destroy();
		lSdkManager = nullptr;
	}

	SetState(ResourceState::Unloaded);
}

size_t Model3D::getSizeInBytes() const
{
	size_t totalSize = 0;
	for (const auto& mesh : m_meshes) {
		totalSize += mesh.m_vertex.size() * sizeof(SimpleVertex);
		totalSize += mesh.m_index.size() * sizeof(unsigned int);
	}
	return totalSize;
}

bool
Model3D::InitializeFBXManager() {
	lSdkManager = FbxManager::Create();
	if (!lSdkManager) {
		ERROR("ModelLoader", "FbxManager::Create()", "Unable to create FBX Manager!");
		return false;
	}

	FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
	lSdkManager->SetIOSettings(ios);

	lScene = FbxScene::Create(lSdkManager, "MyScene");
	if (!lScene) {
		ERROR("ModelLoader", "FbxScene::Create()", "Unable to create FBX Scene!");
		return false;
	}
	return true;
}

std::vector<MeshComponent> 
Model3D::LoadFBXModel(const std::string& filePath) {
	std::vector<MeshComponent> loadedMeshes;

	if (InitializeFBXManager()) {
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");
		if (!lImporter) {
			ERROR("ModelLoader", "FbxImporter::Create()", "Unable to create FBX Importer!");
			return loadedMeshes;
		}

		if (!lImporter->Initialize(filePath.c_str(), -1, lSdkManager->GetIOSettings())) {
			ERROR("ModelLoader", "FbxImporter::Initialize()",
				"Unable to initialize FBX Importer! Error: " << lImporter->GetStatus().GetErrorString());
			lImporter->Destroy();
			return loadedMeshes;
		}

		if (!lImporter->Import(lScene)) {
			ERROR("ModelLoader", "FbxImporter::Import()",
				"Unable to import FBX Scene! Error: " << lImporter->GetStatus().GetErrorString());
			lImporter->Destroy();
			return loadedMeshes;
		}
		else {
			m_name = lImporter->GetFileName();
		}

		FbxAxisSystem::DirectX.ConvertScene(lScene);
		FbxSystemUnit::m.ConvertScene(lScene);
		FbxGeometryConverter gc(lSdkManager);
		gc.Triangulate(lScene, true);

		lImporter->Destroy();

		FbxNode* lRootNode = lScene->GetRootNode();
		if (lRootNode) {
			m_meshes.clear();
			for (int i = 0; i < lRootNode->GetChildCount(); i++) {
				ProcessFBXNode(lRootNode->GetChild(i));
			}
			loadedMeshes = m_meshes;
			return loadedMeshes;
		}
		else {
			ERROR("ModelLoader", "FbxScene::GetRootNode()",
				"Unable to get root node from FBX Scene!");
			return loadedMeshes;
		}
	}
	return loadedMeshes;
}

void 
Model3D::ProcessFBXNode(FbxNode* node) {
	if (node->GetNodeAttribute()) {
		if (node->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh) {
			ProcessFBXMesh(node);
		}
	}

	for (int i = 0; i < node->GetChildCount(); i++) {
		ProcessFBXNode(node->GetChild(i));
	}
}

void 
Model3D::ProcessFBXMesh(FbxNode* node) {
  FbxMesh* mesh = node->GetMesh();
  if (!mesh) return;

  if (mesh->GetElementNormalCount() == 0)
    mesh->GenerateNormals(true, true);

  const char* uvSetName = nullptr;
  {
    FbxStringList uvSets; mesh->GetUVSetNames(uvSets);
    if (uvSets.GetCount() > 0) uvSetName = uvSets[0];
  }

  if (mesh->GetElementTangentCount() == 0 && uvSetName)
    mesh->GenerateTangentsData(uvSetName);

  const FbxGeometryElementUV* uvElem = (mesh->GetElementUVCount() > 0) ? mesh->GetElementUV(0) : nullptr;
  const FbxGeometryElementTangent* tanElem = (mesh->GetElementTangentCount() > 0) ? mesh->GetElementTangent(0) : nullptr;
  const FbxGeometryElementBinormal* binElem = (mesh->GetElementBinormalCount() > 0) ? mesh->GetElementBinormal(0) : nullptr;

  std::vector<SimpleVertex> vertices;
  std::vector<unsigned int> indices;
  vertices.reserve(mesh->GetPolygonCount() * 3);
  indices.reserve(mesh->GetPolygonCount() * 3);

  auto readV2 = [](const FbxGeometryElementUV* elem, int cpIdx, int pvIdx) -> FbxVector2 {
    if (!elem) return FbxVector2(0, 0);
    using E = FbxGeometryElement;
    int idx;
    if (elem->GetMappingMode() == E::eByControlPoint)
      idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(cpIdx) : cpIdx;
    else
      idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(pvIdx) : pvIdx;
    return elem->GetDirectArray().GetAt(idx);
    };
  auto readV4 = [](auto* elem, int cpIdx, int pvIdx) -> FbxVector4 {
    if (!elem) return FbxVector4(0, 0, 0, 0);
    using E = FbxGeometryElement;
    int idx;
    if (elem->GetMappingMode() == E::eByControlPoint)
      idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(cpIdx) : cpIdx;
    else
      idx = (elem->GetReferenceMode() == E::eIndexToDirect) ? elem->GetIndexArray().GetAt(pvIdx) : pvIdx;
    return elem->GetDirectArray().GetAt(idx);
    };

  for (int p = 0; p < mesh->GetPolygonCount(); ++p)
  {
    const int polySize = mesh->GetPolygonSize(p);
    std::vector<unsigned> cornerIdx; cornerIdx.reserve(polySize);

    for (int v = 0; v < polySize; ++v)
    {
      const int cpIndex = mesh->GetPolygonVertex(p, v);
      const int pvIndex = mesh->GetPolygonVertexIndex(p) + v;

      SimpleVertex out{};

      FbxVector4 P = mesh->GetControlPointAt(cpIndex);
      out.Position = { (float)P[0], (float)P[1], (float)P[2] };

      FbxVector4 N(0, 1, 0, 0);
      mesh->GetPolygonVertexNormal(p, v, N);
      N.Normalize();
      out.Normal = { (float)N[0], (float)N[1], (float)N[2] };

      if (uvElem && uvSetName) {
        int uvIdx = mesh->GetTextureUVIndex(p, v);
        FbxVector2 uv = (uvIdx >= 0) ? uvElem->GetDirectArray().GetAt(uvIdx)
          : readV2(uvElem, cpIndex, pvIndex);
        out.TextureCoordinate = { (float)uv[0], 1.0f - (float)uv[1] };
      }
      else {
        out.TextureCoordinate = { 0.0f, 0.0f };
      }

      if (tanElem) {
        FbxVector4 T = readV4(tanElem, cpIndex, pvIndex);
        out.Tangent = { (float)T[0], (float)T[1], (float)T[2] };
      }
      else out.Tangent = { 0,0,0 };
      
      if (binElem) {
        FbxVector4 B = readV4(binElem, cpIndex, pvIndex);
        out.Bitangent = { (float)B[0], (float)B[1], (float)B[2] };
      }
      else out.Bitangent = { 0,0,0 };

      cornerIdx.push_back((unsigned)vertices.size());
      vertices.push_back(out);
    }

    for (int k = 1; k + 1 < polySize; ++k) {
      indices.push_back(cornerIdx[0]);
      indices.push_back(cornerIdx[k + 1]);
      indices.push_back(cornerIdx[k]);
    }
  }

  if (mesh->GetElementTangentCount() == 0 || mesh->GetElementBinormalCount() == 0)
  {
    auto add = [](EU::Vector3 a, const EU::Vector3& b) { a.x += b.x; a.y += b.y; a.z += b.z; return a; };
    auto sub = [](const EU::Vector3& a, const EU::Vector3& b) { return EU::Vector3(a.x - b.x, a.y - b.y, a.z - b.z); };
    auto mul = [](const EU::Vector3& a, float s) { return EU::Vector3(a.x * s, a.y * s, a.z * s); };
  
    for (size_t i = 0; i + 2 < indices.size(); i += 3)
    {
      SimpleVertex& v0 = vertices[indices[i + 0]];
      SimpleVertex& v1 = vertices[indices[i + 1]];
      SimpleVertex& v2 = vertices[indices[i + 2]];
  
      EU::Vector3 e1 = sub(v1.Position, v0.Position);
      EU::Vector3 e2 = sub(v2.Position, v0.Position);
  
      float du1 = v1.TextureCoordinate.x - v0.TextureCoordinate.x;
      float dv1 = v1.TextureCoordinate.y - v0.TextureCoordinate.y;
      float du2 = v2.TextureCoordinate.x - v0.TextureCoordinate.x;
      float dv2 = v2.TextureCoordinate.y - v0.TextureCoordinate.y;
  
      float denom = du1 * dv2 - du2 * dv1;
      float r = (std::fabs(denom) < 1e-8f) ? 0.0f : 1.0f / denom;
  
      EU::Vector3 T = mul(EU::Vector3(e1.x * dv2 - e2.x * dv1, e1.y * dv2 - e2.y * dv1, e1.z * dv2 - e2.z * dv1), r);
      EU::Vector3 B = mul(EU::Vector3(e2.x * du1 - e1.x * du2, e2.y * du1 - e1.y * du2, e2.z * du1 - e1.z * du2), r);
  
      v0.Tangent = add(v0.Tangent, T);
      v1.Tangent = add(v1.Tangent, T);
      v2.Tangent = add(v2.Tangent, T);
      v0.Bitangent = add(v0.Bitangent, B);
      v1.Bitangent = add(v1.Bitangent, B);
      v2.Bitangent = add(v2.Bitangent, B);
    }
  }

  bool autoDetectMirror = true;
  bool forceFlipWinding = true;

  bool mirrored = true;
  if (autoDetectMirror) {
    FbxAMatrix geo;
    geo.SetT(node->GetGeometricTranslation(FbxNode::eSourcePivot));
    geo.SetR(node->GetGeometricRotation(FbxNode::eSourcePivot));
    geo.SetS(node->GetGeometricScaling(FbxNode::eSourcePivot));
    FbxAMatrix world = node->EvaluateGlobalTransform() * geo;

    FbxVector4 S = world.GetS();
    double detScale = S[0] * S[1] * S[2];
    mirrored = (detScale < 0.0);
  }

  if (mirrored || forceFlipWinding) {
    for (size_t i = 0; i + 2 < indices.size(); i += 3)
      std::swap(indices[i + 1], indices[i + 2]);

    for (auto& v : vertices) {
      v.Normal = { v.Normal.x, v.Normal.y, v.Normal.z };
      v.Tangent = { v.Tangent.x, v.Tangent.y, v.Tangent.z };
      v.Bitangent = { v.Bitangent.x, v.Bitangent.y, v.Bitangent.z };
    }
  }

  auto dot3 = [](const EU::Vector3& a, const EU::Vector3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; };
  auto norm3 = [](EU::Vector3& v) { float l = std::sqrt(EU::EMax(1e-20f, v.x * v.x + v.y * v.y + v.z * v.z)); v.x /= l; v.y /= l; v.z /= l; };
  auto sub3 = [](const EU::Vector3& a, const EU::Vector3& b) { return EU::Vector3(a.x - b.x, a.y - b.y, a.z - b.z); };
  auto cross3 = [](const EU::Vector3& a, const EU::Vector3& b) {
    return EU::Vector3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
    };
  
  for (auto& v : vertices)
  {
    norm3(v.Normal);
    float dTN = dot3(v.Tangent, v.Normal);
    v.Tangent = sub3(v.Tangent, EU::Vector3(v.Normal.x * dTN, v.Normal.y * dTN, v.Normal.z * dTN));
    norm3(v.Tangent);
  
    EU::Vector3 Bcalc = cross3(v.Normal, v.Tangent);
    float hand = (dot3(Bcalc, v.Bitangent) < 0.0f) ? -1.0f : 1.0f;
    v.Bitangent = { Bcalc.x * hand, Bcalc.y * hand, Bcalc.z * hand };
    norm3(v.Bitangent);
  }

  MeshComponent mc;
  mc.m_name = node->GetName();
  mc.m_vertex = std::move(vertices);
  mc.m_index = std::move(indices);
  mc.m_numVertex = (int)mc.m_vertex.size();
  mc.m_numIndex = (int)mc.m_index.size();
  m_meshes.push_back(std::move(mc));
}

void Model3D::ProcessFBXMaterials(FbxSurfaceMaterial* material)
{
	if (material) {
		FbxProperty prop = material->FindProperty(FbxSurfaceMaterial::sDiffuse);
		if (prop.IsValid()) {
			int textureCount = prop.GetSrcObjectCount<FbxTexture>();
			for (int i = 0; i < textureCount; ++i) {
				FbxTexture* texture = FbxCast<FbxTexture>(prop.GetSrcObject<FbxTexture>(i));
				if (texture) {
					textureFileNames.push_back(texture->GetName());
				}
			}
		}
	}
}

std::string
Model3D::GetBinaryCachePath() const {
	return m_filePath + ".wvmesh";
}

bool
Model3D::IsBinaryCacheUpToDate(const std::string& sourcePath, const std::string& cachePath) const {
	ULONGLONG sourceWriteTime = 0;
	ULONGLONG cacheWriteTime = 0;

	if (!GetFileWriteTime(sourcePath, sourceWriteTime)) {
		return false;
	}

	if (!GetFileWriteTime(cachePath, cacheWriteTime)) {
		return false;
	}

	return cacheWriteTime >= sourceWriteTime;
}

bool
Model3D::LoadBinaryCache(const std::string& cachePath) {
	std::ifstream stream(cachePath, std::ios::binary);
	if (!stream.is_open()) {
		return false;
	}

	uint32_t magic = 0;
	uint32_t version = 0;
	uint32_t meshCount = 0;
	uint32_t textureCount = 0;

	stream.read(reinterpret_cast<char*>(&magic), sizeof(magic));
	stream.read(reinterpret_cast<char*>(&version), sizeof(version));
	stream.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
	stream.read(reinterpret_cast<char*>(&textureCount), sizeof(textureCount));

	if (!stream.good() || magic != kModelCacheMagic || version != kModelCacheVersion) {
		return false;
	}

	std::vector<MeshComponent> loadedMeshes;
	std::vector<std::string> loadedTextures;
	loadedMeshes.reserve(meshCount);
	loadedTextures.reserve(textureCount);

	for (uint32_t i = 0; i < textureCount; ++i) {
		std::string textureName;
		if (!ReadString(stream, textureName)) {
			return false;
		}
		loadedTextures.push_back(std::move(textureName));
	}

	for (uint32_t i = 0; i < meshCount; ++i) {
		MeshComponent mesh;
		if (!ReadString(stream, mesh.m_name)) {
			return false;
		}

		uint32_t vertexCount = 0;
		uint32_t indexCount = 0;
		stream.read(reinterpret_cast<char*>(&vertexCount), sizeof(vertexCount));
		stream.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
		if (!stream.good()) {
			return false;
		}

		mesh.m_vertex.resize(vertexCount);
		mesh.m_index.resize(indexCount);
		if (vertexCount > 0) {
			stream.read(reinterpret_cast<char*>(mesh.m_vertex.data()), sizeof(SimpleVertex) * vertexCount);
		}
		if (indexCount > 0) {
			stream.read(reinterpret_cast<char*>(mesh.m_index.data()), sizeof(unsigned int) * indexCount);
		}
		if (!stream.good()) {
			return false;
		}

		mesh.m_numVertex = static_cast<int>(vertexCount);
		mesh.m_numIndex = static_cast<int>(indexCount);
		loadedMeshes.push_back(std::move(mesh));
	}

	m_meshes = std::move(loadedMeshes);
	textureFileNames = std::move(loadedTextures);

	const std::wstring cachePathW(cachePath.begin(), cachePath.end());
	MESSAGE("ModelLoader", "BinaryCache",
		L"Loaded binary cache '" << cachePathW << L"'")
	return true;
}

bool
Model3D::SaveBinaryCache(const std::string& cachePath) const {
	std::ofstream stream(cachePath, std::ios::binary | std::ios::trunc);
	if (!stream.is_open()) {
		return false;
	}

	const uint32_t meshCount = static_cast<uint32_t>(m_meshes.size());
	const uint32_t textureCount = static_cast<uint32_t>(textureFileNames.size());

	stream.write(reinterpret_cast<const char*>(&kModelCacheMagic), sizeof(kModelCacheMagic));
	stream.write(reinterpret_cast<const char*>(&kModelCacheVersion), sizeof(kModelCacheVersion));
	stream.write(reinterpret_cast<const char*>(&meshCount), sizeof(meshCount));
	stream.write(reinterpret_cast<const char*>(&textureCount), sizeof(textureCount));

	for (const std::string& textureName : textureFileNames) {
		if (!WriteString(stream, textureName)) {
			return false;
		}
	}

	for (const MeshComponent& mesh : m_meshes) {
		if (!WriteString(stream, mesh.m_name)) {
			return false;
		}

		const uint32_t vertexCount = static_cast<uint32_t>(mesh.m_vertex.size());
		const uint32_t indexCount = static_cast<uint32_t>(mesh.m_index.size());
		stream.write(reinterpret_cast<const char*>(&vertexCount), sizeof(vertexCount));
		stream.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));

		if (vertexCount > 0) {
			stream.write(reinterpret_cast<const char*>(mesh.m_vertex.data()), sizeof(SimpleVertex) * vertexCount);
		}
		if (indexCount > 0) {
			stream.write(reinterpret_cast<const char*>(mesh.m_index.data()), sizeof(unsigned int) * indexCount);
		}

		if (!stream.good()) {
			return false;
		}
	}

	return stream.good();
}
