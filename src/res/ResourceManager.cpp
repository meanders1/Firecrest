#include "ResourceManager.h"
#include "MeshLoader.h"
#include "gl/Texture2D.h"

inline size_t hashMesh(const std::vector<fc::gl::Vertex3D>& vertices,
                       const std::vector<GLuint>& indices) {
    size_t seed = vertices.size() ^ (indices.size() << 1);

    for (const auto& v : vertices) {
        seed ^= std::hash<fc::gl::Vertex3D>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    for (const auto& i : indices) {
        seed ^= std::hash<GLuint>()(i) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    return seed;
}

fc::res::TextureHandle fc::res::ResourceManager::loadTexture(const std::string& path,
                                                             bool blurred) {
    const TextureKey key{path, blurred};

    auto it = _textures.find(key);
    if (it != _textures.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    const auto texture = std::make_shared<gl::Texture2D>(path, blurred);
    _textures[key] = texture;
    return texture;
}

fc::res::ShaderHandle fc::res::ResourceManager::loadShader(const std::string& vertexPath,
                                                           const std::string& fragmentPath) {
    const ShaderKey key{ShaderSourceType::FromFile, vertexPath, fragmentPath};

    auto it = _shaders.find(key);
    if (it != _shaders.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    const auto shader = std::make_shared<gl::Shader>(gl::Shader::fileSource(vertexPath), gl::Shader::fileSource(fragmentPath));
    _shaders[key] = shader;
    return shader;
}

fc::res::ShaderHandle
fc::res::ResourceManager::loadShaderSource(const std::string& vertexSource,
                                           const std::string& fragmentSource) {
    const ShaderKey key{ShaderSourceType::FromString, vertexSource, fragmentSource};

    auto it = _shaders.find(key);
    if (it != _shaders.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    const auto shader = std::make_shared<gl::Shader>();
    shader->addStageSource(GL_VERTEX_SHADER, vertexSource);
    shader->addStageSource(GL_FRAGMENT_SHADER, fragmentSource);
    shader->link();
    _shaders[key] = shader;
    return shader;
}

fc::res::MeshHandle
fc::res::ResourceManager::loadMesh(const std::vector<fc::gl::Vertex3D>& vertices,
                                   const std::vector<GLuint>& indices) {
    const MeshKey key{hashMesh(vertices, indices)};

    auto it = _meshes.find(key);
    if (it != _meshes.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    const auto mesh = std::make_shared<gl::Mesh>(vertices, indices);
    _meshes[key] = mesh;
    return mesh;
}

fc::res::ModelHandle fc::res::ResourceManager::loadModel(const std::string& path) {
    const ModelKey key{path};

    auto it = _models.find(key);
    if (it != _models.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    return res::loadModel(*this, path);
}

fc::res::ModelHandle fc::res::ResourceManager::loadModel(gl::Model&& model,
                                                         const std::string& path) {
    const ModelKey key{path};

    auto it = _models.find(key);
    if (it != _models.end()) {
        if (auto existing = it->second.lock()) {
            return existing;
        }
    }

    const auto handle = std::make_shared<gl::Model>(std::move(model));
    _models[key] = handle;
    return handle;
}
