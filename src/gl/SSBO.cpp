#include "SSBO.h"

namespace fc::gl {
SSBO::SSBO(const SSBOLayout& ssboLayout)
{
    setLayout(ssboLayout);
}

void SSBO::setLayout(const SSBOLayout& layout)
{
    _layout = layout;
    GLsizeiptr size = static_cast<GLsizeiptr>(layout.getTotalSize());
    bind();
    Buffer::setData(nullptr, size, GL_DYNAMIC_DRAW);
    unbind();
}

void* SSBO::dataPointer(GLuint index, GLbitfield access)
{
    GLintptr offset = _layout.getOffset(index);
    GLsizeiptr size = _layout.getComponent(index).getSize();
    return Buffer::dataPointer(offset, size, access);
}

void SSBO::getData(void* dest, GLuint index)
{
    GLintptr offset = _layout.getOffset(index);
    GLsizeiptr size = _layout.getComponent(index).getSize();
    Buffer::getData(dest, offset, size);
}

void SSBO::setData(const void* data, GLuint index)
{
    GLintptr offset = _layout.getOffset(index);
    GLsizeiptr size = _layout.getComponent(index).getSize();
    editData(data, offset, size);
}

void SSBO::bindIndex(GLuint index) const
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, _handle.id);
}

bool SSBO::resizeLast(GLuint count)
{
    bool changed = _layout.resizeLast(count);
    if (changed) {
        GLsizeiptr size = _layout.getTotalSize();
        Buffer::setData(nullptr, size, GL_DYNAMIC_DRAW);
    }
    return changed;
}

} // namespace fc::gl