#include "VertexArray.h"
#include "VertexBufferLayout.h"

namespace fc::gl {

void VertexArray::addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) const
{
    bind();
    vb.bind();
    const auto& elements = layout.elements();

    for (GLuint i = 0; i < elements.size(); i++) {
        const VertexBufferElement& element = elements[i];
        glEnableVertexAttribArray(i);

        if (VertexBufferElement::isInteger(element.type)) {
            glVertexAttribIPointer(i, element.count, element.type, layout.getStride(),
                                   reinterpret_cast<const void*>(element.offset));
        }
        else {
            glVertexAttribPointer(i, element.count, element.type, element.normalized,
                                  layout.getStride(),
                                  reinterpret_cast<const void*>(element.offset));
        }
    }
}

void VertexArray::addBuffer(const IndexBuffer& ib) const
{
    ib.bind();
}

void VertexArray::bind() const
{
    glBindVertexArray(_handle.id);
}

void VertexArray::unbind() const
{
    glBindVertexArray(0);
}

} // namespace fc::gl
