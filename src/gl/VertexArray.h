#pragma once

#include "IndexBuffer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"

namespace fc::gl {

struct VertexArrayHandle {
    GLuint id = 0;

    VertexArrayHandle() { glGenVertexArrays(1, &id); }

    ~VertexArrayHandle()
    {
        if (id)
            glDeleteVertexArrays(1, &id);
    }

    VertexArrayHandle(const VertexArrayHandle&) = delete;
    VertexArrayHandle& operator=(const VertexArrayHandle&) = delete;

    VertexArrayHandle(VertexArrayHandle&& other) noexcept : id(other.id) { other.id = 0; }

    VertexArrayHandle& operator=(VertexArrayHandle&& other) noexcept
    {
        if (this != &other) {
            if (id)
                glDeleteVertexArrays(1, &id);
            id = other.id;
            other.id = 0;
        }
        return *this;
    }

    bool valid() const { return id != 0; }
};

class VertexArray {
private:
    VertexArrayHandle _handle;

public:
    VertexArray() = default;

    VertexArray& operator=(VertexArray&& other) noexcept = default;
    VertexArray(VertexArray&& other) noexcept = default;

    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;

    void addBuffer(const VertexBuffer& vb, const VertexBufferLayout& layout) const;
    void addBuffer(const IndexBuffer& ib) const;

    void bind() const;
    void unbind() const;

    const VertexArrayHandle& handle() const { return _handle; }
};

} // namespace fc::gl