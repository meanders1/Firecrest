#pragma once
#include "OpenGL.h"
#include <algorithm>

namespace fc::gl {

struct BufferHandle {
    GLuint id = 0;

    BufferHandle() { glGenBuffers(1, &id); }

    ~BufferHandle()
    {
        if (id) {
            glDeleteBuffers(1, &id);
        }
    }

    BufferHandle(const BufferHandle&) = delete;
    BufferHandle& operator=(const BufferHandle&) = delete;

    BufferHandle(BufferHandle&& o) noexcept : id(o.id) { o.id = 0; }

    BufferHandle& operator=(BufferHandle&& o) noexcept
    {
        if (this != &o) {
            if (id)
                glDeleteBuffers(1, &id);
            id = o.id;
            o.id = 0;
        }
        return *this;
    }

    bool valid() const { return id != 0; }
};

template <GLenum tType>
class Buffer {
protected:
    BufferHandle _handle;

    GLsizeiptr _size;

public:
    Buffer() = default;

    Buffer& operator=(Buffer&& other) noexcept = default;
    Buffer(Buffer&& other) noexcept = default;

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    inline void bind() const { glBindBuffer(tType, _handle.id); }

    inline void unbind() const { glBindBuffer(tType, 0); }

    void setData(const void* data) { setData(data, 0, _size); }

    void editData(const void* data, GLintptr offset, GLsizeiptr size)
    {
        bind();
        glBufferSubData(tType, offset, size, data);
        unbind();
    }

    void setData(const void* data, GLsizeiptr size, GLenum usage)
    {
        bind();
        glBufferData(tType, size, data, usage);
        _size = size;
        unbind();
    }

    void getData(void* dest) const { getData(dest, 0, _size); }

    void getData(void* dest, GLintptr offset, GLsizeiptr size) const
    {
        bind();
        glGetBufferSubData(tType, offset, size, dest);
        unbind();
    }

    void* dataPointer(GLbitfield access) { return dataPointer(0, _size, access); }

    void* dataPointer(GLintptr offset, GLsizeiptr size, GLbitfield access)
    {
        bind();
        void* ptr = glMapBufferRange(tType, offset, size, access);
        return ptr;
    }

    void close()
    {
        glUnmapBuffer(tType);
        unbind();
    }

    inline const BufferHandle& getHandle() const { return _handle; }
    inline GLsizeiptr getSize() const { return _size; }
};
} // namespace fc::gl