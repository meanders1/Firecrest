#pragma once
#include "Buffer.h"
#include "SSBOLayout.h"
#include <cstddef>
#include <vector>

namespace fc::gl {

class SSBO : public Buffer<GL_SHADER_STORAGE_BUFFER> {
private:
    SSBOLayout _layout;

    void setLayout(const SSBOLayout& layout);

public:
    const SSBOLayout& getLayout() const { return _layout; }

public:
    SSBO(const SSBOLayout& ssboLayout);

    SSBO(const SSBO&) = delete;
    SSBO& operator=(const SSBO&) = delete;

    SSBO(SSBO&&) noexcept = default;
    SSBO& operator=(SSBO&&) noexcept = default;

    void* dataPointer(GLuint index, GLbitfield access);

    void getData(void* dest, GLuint index);

    void setData(const void* data, GLuint index);

    // Must be called before rendering/compute shader dispatch to associate this
    // buffer with the one in the shader
    void bindIndex(GLuint index) const;

    /// Returns true if the size was changed, else false.
    bool resizeLast(GLuint count);
};
} // namespace fc::gl
