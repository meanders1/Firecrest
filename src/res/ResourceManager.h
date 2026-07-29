#pragma once

#include "gl/OpenGL.h"
#include "gl/Vertex3D.h"
#include "glm/glm.hpp"
#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"

#include "gl/Mesh.h"
#include "gl/Model.h"
#include "gl/Shader.h"
#include "gl/Texture.h"

#include <memory>

namespace fc {
struct TextureKey {
    const std::string path;
    const bool blurred;

    TextureKey(const std::string& p, bool b) : path(p), blurred(b) {}

    bool operator==(const TextureKey& other) const {
        return path == other.path && blurred == other.blurred;
    }
};

enum class ShaderSourceType { FromFile, FromString };

struct ShaderKey {
    ShaderSourceType sourceType;
    std::string vertexSource;
    std::string fragmentSource;

    ShaderKey(ShaderSourceType type, const std::string& vs, const std::string& fs)
        : sourceType(type), vertexSource(vs), fragmentSource(fs) {}

    bool operator==(const ShaderKey& other) const {
        return sourceType == other.sourceType && vertexSource == other.vertexSource
               && fragmentSource == other.fragmentSource;
    }
};

struct MeshKey {
    const size_t hash;

    bool operator==(const MeshKey& other) const { return hash == other.hash; }
};

struct ModelKey {
    const std::string path;

    ModelKey(const std::string& filepath) : path(filepath) {}

    bool operator==(const ModelKey& other) const { return path == other.path; }
};
} // namespace fc

namespace std {
template <> struct hash<fc::TextureKey> {
    size_t operator()(const fc::TextureKey& key) const {
        size_t h1 = std::hash<std::string>()(key.path);
        size_t h2 = std::hash<bool>()(key.blurred);
        return h1 ^ (h2 << 1);
    }
};

template <> struct hash<fc::ShaderKey> {
    std::size_t operator()(const fc::ShaderKey& key) const {
        std::size_t h1 = std::hash<int>()(static_cast<int>(key.sourceType));
        std::size_t h2 = std::hash<std::string>()(key.vertexSource);
        std::size_t h3 = std::hash<std::string>()(key.fragmentSource);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

template <> struct hash<fc::MeshKey> {
    size_t operator()(const fc::MeshKey& key) const { return key.hash; }
};

template <> struct hash<fc::ModelKey> {
    std::size_t operator()(const fc::ModelKey& key) const {
        return std::hash<std::string>()(key.path);
    }
};

template <> struct hash<glm::vec2> {
    size_t operator()(const glm::vec2& v) const {
        size_t h1 = std::hash<float>()(v.x);
        size_t h2 = std::hash<float>()(v.y);
        return h1 ^ (h2 << 1);
    }
};

template <> struct hash<glm::vec3> {
    size_t operator()(const glm::vec3& v) const {
        size_t h1 = std::hash<float>()(v.x);
        size_t h2 = std::hash<float>()(v.y);
        size_t h3 = std::hash<float>()(v.z);
        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

template <> struct hash<fc::gl::Vertex3D> {
    size_t operator()(const fc::gl::Vertex3D& v) const {
        size_t h1 = std::hash<glm::vec3>()(v.position);
        size_t h2 = std::hash<glm::vec2>()(v.texCoord);
        size_t h3 = std::hash<glm::vec3>()(v.normal);
        size_t h4 = std::hash<glm::vec3>()(v.tangent);

        size_t seed = h1;
        seed ^= (h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        seed ^= (h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        seed ^= (h4 + 0x9e3779b9 + (seed << 6) + (seed >> 2));
        return seed;
    }
};
} // namespace std

namespace fc::res {

class ResourceManager {
public:
    ResourceManager() = default;

    // Load a texture from file
    TextureHandle loadTexture(const std::string& path, bool blurred = false);
    // Load a shader from file
    ShaderHandle loadShader(const std::string& vertexPath, const std::string& fragmentPath);
    // Load a shader from source strings
    ShaderHandle loadShaderSource(const std::string& vertexSource,
                                  const std::string& fragmentSource);
    // Load a mesh from vertices and indices
    MeshHandle loadMesh(const std::vector<gl::Vertex3D>& vertices,
                        const std::vector<GLuint>& indices);
    // Load a model from file
    ModelHandle loadModel(const std::string& path);

    // Load a model from memory. Note! this moves the model supplied
    ModelHandle loadModel(gl::Model&& model, const std::string& path);

private:
    std::unordered_map<TextureKey, std::weak_ptr<gl::Texture2D>> _textures;
    std::unordered_map<ShaderKey, std::weak_ptr<gl::Shader>> _shaders;
    std::unordered_map<MeshKey, std::weak_ptr<gl::Mesh>> _meshes;
    std::unordered_map<ModelKey, std::weak_ptr<gl::Model>> _models;
};
} // namespace fc::res
