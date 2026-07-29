#include "Model.h"
#include "RenderRegion.h"
#include "Texture2D.h"
#include "glm/gtc/type_ptr.hpp"
#include <algorithm>

namespace fc::gl {

void Model::render(const Window& window, const Camera& camera, const Light& light)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    const Rectangle viewport = gl::RenderRegion::currentViewport();

    shader->bind();
    shader->setUniformMat4f("u_View", camera.viewMatrix());
    shader->setUniformMat4f("u_Projection",
                            camera.projectionMatrix(viewport.width / viewport.height));
    shader->setUniformMat4f("u_Transform", transform);
    const glm::vec3 cameraPos = camera.getPosition();
    shader->setUniform3fv("u_CamPos", glm::value_ptr(cameraPos));
    for (auto& [mesh, material] : subMeshes) {

        mesh->vao.bind();

        shader->setUniform1f("u_Material.shininess", material.shininess);
        shader->setUniform1f("u_Material.transparency", material.transparency);

        if (material.diffuseTexture) {
            material.diffuseTexture->bind(1);
            shader->setUniform1i("u_Material.diffuseTex", 1);
        }
        else {
            shader->setUniform3fv("u_Material.ambientColor", &material.ambientColor[0]);
            shader->setUniform3fv("u_Material.diffuseColor", &material.diffuseColor[0]);
            // shader.setUniform1i("u_Material.diffuseTex", 0);
        }

        if (material.specularMap) {
            material.specularMap->bind(2);
            shader->setUniform1i("u_Material.specularTex", 2);
            shader->setUniform1i("u_Material.useSpecularTex", true);
        }
        else {
            if (shader->uniformExists("u_Material.useSpecularTex")) {
                shader->setUniform1i("u_Material.useSpecularTex", false);
            }
            shader->setUniform3fv("u_Material.specularColor", &material.specularColor[0]);
            // shader.setUniform1i("u_Material.specularTex", 0);
        }

        if (material.normalMap) {
            material.normalMap->bind(3);
            shader->setUniform1i("u_Material.normalTex", 3);
            shader->setUniform1i("u_Material.useNormalTex", true);
        }
        // else {
        // shader.setUniform1i("u_Material.normalTex", 0);
        //}

        shader->setUniform3fv("u_Light.position", &light.position[0]);
        shader->setUniform3fv("u_Light.ambient", &light.ambient[0]);
        shader->setUniform3fv("u_Light.diffuse", &light.diffuse[0]);
        shader->setUniform3fv("u_Light.specular", &light.specular[0]);

        shader->setUniform4fv("u_Material.overrideColor", &material.overrideColor[0]);

        GLsizei amtIndices = mesh->ibo.getCount();
        mesh->ibo.bind();
        glDrawElements(GL_TRIANGLES, amtIndices, GL_UNSIGNED_INT, nullptr);
    }
}

Model& Model::operator=(Model&& other)
{
    std::swap(shader, other.shader);
    subMeshes.swap(other.subMeshes);
    return *this;
}

Model::Model(Model&& other)
{
    shader = std::move(other.shader);
    subMeshes = std::move(other.subMeshes);
}
} // namespace fc::gl
