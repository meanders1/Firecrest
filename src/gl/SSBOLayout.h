#pragma once

#include <stdexcept>
#include <vector>

namespace fc::gl {

class SSBOComponent {
private:
    GLsizeiptr _elementSize; // The number of bytes per element
    GLuint _count;           // The number of elements
    bool _fixedSize;

public:
    SSBOComponent(GLsizeiptr elementSize, GLuint count, bool isFixedSize)
        : _elementSize(elementSize), _count(count), _fixedSize(isFixedSize) {
        if (count < 0)
            _fixedSize = false;
    }

    inline GLsizeiptr getSize() const { return _count * _elementSize; }

    inline GLuint getCount() const { return _count; }

    inline bool isFixedSize() const { return _fixedSize; }

    // Returns true if the size was changed, else false.
    bool resize(GLuint count) {
        if (!_fixedSize) {
            GLuint oldCount = _count;
            _count = count;
            return oldCount != _count;
        }
        return false;
    }
};

class SSBOLayout {
private:
    std::vector<SSBOComponent> components;

public:
    SSBOLayout() {}

    GLintptr getOffset(GLuint index) const {
        GLintptr offset = 0;
        for (GLintptr i = 0; i < index; i++) {
            offset += components[i].getSize();
        }
        return offset;
    }

    template <typename T> void addComponent(GLuint count) {
        if (components.size() > 0 && !components[components.size() - 1].isFixedSize()) {
            throw std::invalid_argument("Can not have a SSBOComponent after a "
                                        "SSBOComponent with undefined length");
            return;
        }
        components.emplace_back(sizeof(T), count, true);
    }

    template <typename T> void addVariableSizedComponent(GLuint initialCount) {
        if (components.size() > 0 && !components[components.size() - 1].isFixedSize()) {
            throw std::invalid_argument("Can not have a SSBOComponent after a "
                                        "SSBOComponent with undefined length");
            return;
        }
        components.emplace_back(sizeof(T), initialCount, false);
    }

    /// Returns true if the size was changed, else false.
    bool resizeLast(GLuint count) {
        if (components.size() == 0) {
            throw std::out_of_range("Can not resize last component of an empty SSBOLayout");
            return false;
        }
        if (components[components.size() - 1].isFixedSize()) {
            throw std::logic_error("Can not resize a fixed size SSBOComponent");
            return false;
        }
        return components[components.size() - 1].resize(count);
    }

    inline GLsizeiptr getTotalSize() const {
        GLsizeiptr size = 0;
        for (GLuint i = 0; i < components.size(); i++) {
            size += components[i].getSize();
        }
        return size;
    }

    inline SSBOComponent& getComponent(GLuint index) { return components[index]; }

    inline const SSBOComponent& getComponent(GLuint index) const { return components[index]; }
};

} // namespace fc::gl
