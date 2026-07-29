#include "MeshLoader.h"

#include <algorithm>
#include <array>
#include <execution>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>

#include "core/StringUtils.h"
#include "core/Time.h"

#include "res/ResourceManager.h"

static constexpr const char* TEXTURED_VERTEX_SOURCE = R"(
#version 450 core 
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;
layout(location = 3) in vec3 a_Tangent;

uniform mat4 u_Transform;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_TexCoord;
out vec3 v_FragPos;
out vec3 v_Normal;
out mat3 v_TBN;

void main() {
	vec3 T = normalize(vec3(u_Transform * vec4(a_Tangent,   0.0)));
	vec3 N = normalize(vec3(u_Transform * vec4(a_Normal,    0.0)));
	vec3 B = cross(N, T);
	v_TBN = mat3(T, B, N);

	v_TexCoord = a_TexCoord;
	v_FragPos = a_Position;//vec3(u_Transform * vec4(a_Position, 1.0f));
	v_Normal = a_Normal;
	mat4 mvp = u_Projection * u_View * u_Transform;
	gl_Position = mvp * vec4(a_Position, 1.0);
}
)";

static constexpr const char* TEXTURED_FRAGMENT_SOURCE = R"(
#version 330 core

layout(location = 0) out vec4 fragColor;

in vec2 v_TexCoord;
in vec3 v_FragPos;
in vec3 v_Normal;
in mat3 v_TBN;

struct Material {
	sampler2D diffuseTex;
	
	bool useSpecularTex;
	sampler2D specularTex;
	
	bool useNormalTex;
	sampler2D normalTex;

	// In case no specular and diffuse maps
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;

	float shininess;
	float transparency;

	vec4 overrideColor;
};

struct Light {
	vec3 position;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material u_Material;
uniform Light u_Light;
uniform vec3 u_CamPos;

void main() {
	vec3 normal = normalize(v_Normal);
	if(u_Material.useNormalTex) {
		normal = texture(u_Material.normalTex, v_TexCoord).rgb;
		normal = normal * 2.0 - 1.0;
		normal = normalize(v_TBN * normal);
	}
	vec3 ambient = u_Light.ambient * texture(u_Material.diffuseTex, v_TexCoord).rgb;
	
	vec3 lightDir = normalize(u_Light.position - v_FragPos);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = u_Light.diffuse * diff * texture(u_Material.diffuseTex, v_TexCoord).rgb;
	
	vec3 viewDir = normalize(u_CamPos - v_FragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), u_Material.shininess);
	vec3 specular = u_Light.specular * spec;
	
	if(u_Material.useSpecularTex) {
		specular *= texture(u_Material.specularTex, v_TexCoord).rgb;
	} else {
		specular *= u_Material.specularColor;
	}

	vec3 result = ambient + diffuse + specular;
	fragColor = vec4(result, u_Material.transparency);

	if(u_Material.overrideColor.x >= 0) { 
		// An override color with negative values means no override color
		fragColor = u_Material.overrideColor;
	}
}
)";

static constexpr const char* NONTEXTURED_VERTEX_SOURCE = R"(
#version 450 core 
layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec2 a_TexCoord;
layout(location = 2) in vec3 a_Normal;

uniform mat4 u_Transform;
uniform mat4 u_View;
uniform mat4 u_Projection;

out vec2 v_TexCoord;
out vec3 v_FragPos;
out vec3 v_Normal;

void main() {
	v_TexCoord = a_TexCoord;
	v_FragPos = vec3(u_Transform * vec4(a_Position, 1.0f));
	v_Normal = a_Normal;
	mat4 mvp = u_Projection * u_View * u_Transform;
	gl_Position = mvp * vec4(a_Position, 1.0);
}
)";

static constexpr const char* NONTEXTURED_FRAGMENT_SOURCE = R"(
#version 330 core

layout(location = 0) out vec4 fragColor;

in vec2 v_TexCoord;
in vec3 v_FragPos;
in vec3 v_Normal;

struct Material {
	vec3 ambientColor;
	vec3 diffuseColor;
	vec3 specularColor;
	float shininess;
	float transparency;
 
	vec4 overrideColor;
};

struct Light {
	vec3 position;
	
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Material u_Material;

uniform Light u_Light;
uniform vec3 u_CamPos;

void main() {
    vec3 normal = normalize(v_Normal);
    vec3 lightColor = u_Material.diffuseColor;
	
	// ambient
    vec3 ambient = u_Material.ambientColor * u_Light.ambient;
    
	// diffuse
    vec3 lightDir = normalize(u_Light.position - v_FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * u_Material.diffuseColor * u_Light.diffuse;

    // specular
    vec3 viewDir = normalize(u_CamPos - v_FragPos);
    float spec = 0.0;
	if(u_Material.shininess != 0) {
		vec3 reflectDir = reflect(-lightDir, normal);
		
		vec3 halfwayDir = normalize(lightDir + u_CamPos);  
		spec = pow(max(dot(normal, halfwayDir), 0.0), u_Material.shininess);
		float diffEase = 1 - pow(1 - diff, 3);
		spec *= diffEase;
	}
	vec3 specular = spec * u_Material.specularColor * u_Light.specular;   

	fragColor = vec4(ambient + diffuse + specular, u_Material.transparency);

	if(u_Material.overrideColor.x >= 0) { // An override color with negative values means no override color
		fragColor = u_Material.overrideColor;
	}
}
)";

namespace fc {
namespace res {
using namespace utils;

static void trim(std::vector<std::string>& vec)
{
    for (std::string& str : vec) {
        utils::trim(str);
    }
}

// searchCoverage:
//    0: search just the last vertex,
//	  1: search the whole array
// returns -1 if no match is found, else returns the index of the first match
template <typename T, typename F>
static int64_t findEqual(const std::vector<T>& vec, const T& obj, F&& checkFunction, int max,
                         float searchCoverage = 0.1)
{

    int min = static_cast<int>((1 - searchCoverage) * vec.size());
    // Keep max within bounds of vec
    if (max < 0 || max > static_cast<int>(vec.size() - 1)) {
        max = static_cast<int>(vec.size() - 1);
    }
    for (int i = max; i >= min; i--) {
        if (checkFunction(vec[i], obj)) {
            return i;
        }
    }
    return -1;
}

static void addVertex(std::vector<gl::Vertex3D>& vertices, std::vector<GLuint>& indices,
                      gl::Vertex3D& vertex)
{
    vertices.push_back(vertex);
    indices.push_back(static_cast<GLuint>(vertices.size() - 1));
}

static std::vector<GLuint> createIndices(std::vector<gl::Vertex3D>& vertices,
                                         float vertexSearchCoverage)
{
    std::vector<gl::Vertex3D> newVertices;
    std::vector<GLuint> newIndices;
    for (uint32_t i = 0; i < vertices.size(); i++) {
        gl::Vertex3D& vertex = vertices[i];
        int64_t found = -1;
        if (newVertices.size() != 0) {
            found = findEqual(
                newVertices, vertex,
                [](const gl::Vertex3D& v0, const gl::Vertex3D& v1) -> bool {
                    return v0.position == v1.position && v0.texCoord == v1.texCoord
                           && v0.normal == v1.normal;
                },
                i - 1, vertexSearchCoverage);
        }

        if (found != -1) {
            newIndices.push_back(static_cast<GLuint>(found));
        }
        else {
            newVertices.push_back(vertex);
            newIndices.push_back(static_cast<GLuint>(newVertices.size() - 1));
        }
    }
    vertices = newVertices;
    return newIndices;
}

static res::MeshHandle createMesh(ResourceManager& res, std::vector<gl::Vertex3D>& vertices,
                                  float vertexSearchCoverage)
{
    std::vector<GLuint> indices = createIndices(vertices, vertexSearchCoverage);
    // Create tangents
    for (uint32_t i = 0; i < indices.size(); i += 3) {
        gl::Vertex3D& v0 = vertices[indices[i + 0]];
        gl::Vertex3D& v1 = vertices[indices[i + 1]];
        gl::Vertex3D& v2 = vertices[indices[i + 2]];

        glm::vec3 edge1 = v1.position - v0.position;
        glm::vec3 edge2 = v2.position - v0.position;

        float deltaU1 = v1.texCoord.x - v0.texCoord.x;
        float deltaV1 = v1.texCoord.y - v0.texCoord.y;
        float deltaU2 = v2.texCoord.x - v0.texCoord.x;
        float deltaV2 = v2.texCoord.y - v0.texCoord.y;

        float f = 1.0f / (deltaU1 * deltaV2 - deltaU2 * deltaV1);

        glm::vec3 tangent = f * (deltaV2 * edge1 - deltaV1 * edge2);

        v0.tangent += tangent;
        v1.tangent += tangent;
        v2.tangent += tangent;
    }

    for (gl::Vertex3D& vert : vertices) {
        vert.tangent = glm::normalize(vert.tangent);
    }

    return res.loadMesh(vertices, indices);
}

std::unordered_map<std::string, gl::Material> loadMaterialLib(ResourceManager& res,
                                                              const std::string& path);

// Loads an .obj file
ModelHandle loadModel(ResourceManager& res, const std::string& modelPath,
                      float vertexSearchCoverage)
{
    std::cout << "Loading model: " << modelPath;
    time::Moment startTime = time::now();
    gl::Model model;

    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texCoords;

    std::vector<gl::Vertex3D> vertices;
    std::vector<GLuint> indices;

    std::unordered_map<std::string, gl::Material> materials;
    std::string currentMaterial;

    std::ifstream file(modelPath);
    if (!file.good()) {
        throw std::invalid_argument("Could not load model. File does not exist: \"" + modelPath
                                    + "\"");
    }
    std::string line;

    while (getline(file, line)) {
        line = whitespaceToSpace(line);
        size_t commentStart = line.find("#");
        if (commentStart != std::string::npos) {
            line = line.substr(0, commentStart);
        }
        utils::trim(line);
        std::vector<std::string> tokens = strsplit(line, " ");
        if (tokens.size() == 0 || line.empty())
            continue;

        if (tokens[0] == "v") {
            positions.push_back({std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3])});
        }
        else if (tokens[0] == "vn") {
            normals.push_back({std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3])});
        }
        else if (tokens[0] == "vt") {
            texCoords.push_back({std::stof(tokens[1]), std::stof(tokens[2])});
        }
        else if (tokens[0] == "f") {
            std::vector<gl::Vertex3D> points;

            for (uint32_t i = 1; i < tokens.size(); i++) {
                std::vector<std::string> vertexData = strsplit(tokens[i], "/");
                gl::Vertex3D vertex;

                const uint32_t pos = std::stoi(vertexData[0]);
                vertex.position = positions[pos > 0 ? pos - 1 : positions.size() - pos];

                switch (vertexData.size()) {
                case 1: {
                    vertex.texCoord = {0, 0};
                    vertex.normal = {0, 0, 0};
                    break;
                }
                case 2: {
                    const uint32_t tex = std::stoi(vertexData[1]);
                    vertex.texCoord = texCoords[tex > 0 ? tex - 1 : texCoords.size() - tex];
                    vertex.normal = {0, 0, 0};
                    break;
                }
                case 3: {
                    // must be protected against: f v1//vn1 v2//vn2 v3//vn3
                    const uint32_t normal = std::stoi(vertexData[2]);
                    if (!vertexData[1].empty()) {
                        const uint32_t tex = std::stoi(vertexData[1]);
                        vertex.texCoord = texCoords[tex > 0 ? tex - 1 : texCoords.size() - tex];
                    }
                    else {
                        vertex.texCoord = glm::vec2{0, 0};
                    }
                    vertex.normal = normals[normal > 0 ? normal - 1 : normals.size() - normal];
                    break;
                }
                }
                points.push_back(vertex);
            }
            for (uint32_t i = 1; i < points.size() - 1; i++) {
                addVertex(vertices, indices, points[0]);
                addVertex(vertices, indices, points[i]);
                addVertex(vertices, indices, points[i + 1]);
            }
        }
        else if (tokens[0] == "mtllib") {
            std::string path = getPath(modelPath);

            std::string fileName = tokens[1];
            if (tokens.size() > 2) {
                for (uint32_t i = 2; i < tokens.size(); i++) {
                    fileName += " " + tokens[i];
                }
            }

            path += fileName;
            materials = loadMaterialLib(res, path);
        }
        else if (tokens[0] == "usemtl") {
            if (tokens[1] == currentMaterial)
                continue;

            if (vertices.size() != 0) {
                res::MeshHandle mesh = createMesh(res, vertices, vertexSearchCoverage);
                gl::Material material;
                if (!materials.empty()) {
                    material = materials[currentMaterial];
                }
                model.subMeshes.push_back({mesh, material});
                vertices.clear();
                indices.clear();
            }
            currentMaterial = tokens[1];
        }
        else if (tokens[0] == "o") {
            // Not planning on implementing named objects
        }
        else if (tokens[0] == "s") {
            // Maybe implement later
        }
        else {
            std::cout << "\tLine ignored: \"" << line << "\"" << std::endl;
        }
    }
    file.close();
    if (model.subMeshes.empty()) {
        res::MeshHandle mesh = createMesh(res, vertices, vertexSearchCoverage);
        if (materials.empty()) {
            model.subMeshes.push_back({mesh, gl::Material()});
        }
        else {
            model.subMeshes.push_back({mesh, materials.begin()->second});
        }
    }

    auto texturedShader = res.loadShaderSource(TEXTURED_VERTEX_SOURCE, TEXTURED_FRAGMENT_SOURCE);
    auto nontexturedShader
        = res.loadShaderSource(NONTEXTURED_VERTEX_SOURCE, NONTEXTURED_FRAGMENT_SOURCE);

    // Assign correct shader
    if (model.subMeshes[0].second.diffuseTexture) {
        model.shader = texturedShader;
    }
    else {
        model.shader = nontexturedShader;
    }

    uint32_t amtVertices = 0;
    uint32_t amtIndices = 0;
    for (auto& submesh : model.subMeshes) {
        const res::MeshHandle& mesh = submesh.first;
        amtVertices += static_cast<uint32_t>(mesh->vbo.getSize() / sizeof(gl::Vertex3D));
        amtIndices += static_cast<uint32_t>(mesh->ibo.getCount());
    }
    std::cout << "\tMesh count: " << model.subMeshes.size() << std::endl;
    std::cout << "\tAmount of vertices avoided: " << (1.0f - (float)amtVertices / amtIndices) * 100
              << "% (" << amtVertices << " vertices, " << amtIndices << " indices, "
              << (int)(vertexSearchCoverage * 100) << "% search coverage)"
              << " Time: " << (time::now() - startTime).millis() << "ms" << std::endl;

    return res.loadModel(std::move(model), modelPath);
}

std::unordered_map<std::string, gl::Material> loadMaterialLib(ResourceManager& res,
                                                              const std::string& path)
{
    std::ifstream file(path);
    std::string line;
    if (!file.good()) {
        throw std::invalid_argument("Could not load material file. File does not exist: \"" + path
                                    + "\"");
        return std::unordered_map<std::string, gl::Material>{};
    }
    std::unordered_map<std::string, gl::Material> materials;
    std::string currentMaterial = "";
    std::unordered_map<std::string, std::string> diffuseTexNames;
    std::unordered_map<std::string, std::string> specularTexNames;
    std::unordered_map<std::string, std::string> normalTexNames;

    while (getline(file, line)) {
        size_t commentStart = line.find("#");
        if (commentStart != std::string::npos) {
            line = line.substr(0, commentStart);
        }
        line = whitespaceToSpace(line);
        utils::trim(line);
        if (line.empty())
            continue;
        std::vector<std::string> tokens = strsplit(line, " ");

        if (tokens[0] == "newmtl") {
            materials[tokens[1]] = gl::Material();
            currentMaterial = tokens[1];
        }
        else if (tokens[0] == "Ka") {
            materials[currentMaterial].ambientColor
                = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
        }
        else if (tokens[0] == "Kd") {
            materials[currentMaterial].diffuseColor
                = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
        }
        else if (tokens[0] == "Ks") {
            materials[currentMaterial].specularColor
                = glm::vec3(std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]));
        }
        else if (tokens[0] == "Ns") {
            materials[currentMaterial].shininess = std::stof(tokens[1]);
        }
        else if (tokens[0] == "d") {
            materials[currentMaterial].transparency = std::stof(tokens[1]);
        }
        else if (tokens[0] == "Tr") {
            materials[currentMaterial].transparency = 1 - std::stof(tokens[1]);
        }
        else if (tokens[0] == "map_Kd" || tokens[0] == "map_Ka") {
            std::string fileName = tokens[1];
            if (tokens.size() > 2) {
                for (uint32_t i = 2; i < tokens.size(); i++) {
                    fileName += " " + tokens[i];
                }
            }
            diffuseTexNames[currentMaterial] = fileName;
        }
        else if (tokens[0] == "map_Ks") {
            std::string fileName = tokens[1];
            if (tokens.size() > 2) {
                for (uint32_t i = 2; i < tokens.size(); i++) {
                    fileName += " " + tokens[i];
                }
            }
            specularTexNames[currentMaterial] = fileName;
        }
        else if (tokens[0] == "map_Bump" || tokens[0] == "map_bump") {
            std::string fileName = tokens[1];
            if (tokens.size() > 2) {
                for (uint32_t i = 2; i < tokens.size(); i++) {
                    fileName += " " + tokens[i];
                }
            }
            normalTexNames[currentMaterial] = fileName;
        }
        else {
            std::cout << "\tLine ignored in material file: \"" << line << "\"" << std::endl;
        }
    }
    for (auto& texture : diffuseTexNames) {
        materials[texture.first].diffuseTexture
            = res.loadTexture(getPath(path) + texture.second, true);
    }
    for (auto& texture : specularTexNames) {
        materials[texture.first].specularMap
            = res.loadTexture(getPath(path) + texture.second, true);
    }
    for (auto& texture : normalTexNames) {
        materials[texture.first].normalMap = res.loadTexture(getPath(path) + texture.second, true);
    }
    return materials;
}

} // namespace res
} // namespace fc