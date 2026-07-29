#include "Shader.h"
#include "core/StringUtils.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

namespace fc::gl {

Shader::Shader(const std::string& vertexSource, const std::string& fragmentSource)
{
    addStageSource(GL_VERTEX_SHADER, vertexSource);
    addStageSource(GL_FRAGMENT_SHADER, fragmentSource);
    link();
}

void Shader::addStage(const GLenum shaderType, const std::string& filePath)
{
    std::string source = fileSource(filePath);
    addStageSource(shaderType, source);
}

void Shader::addStageSource(const GLenum shaderType, const std::string& source)
{
    GLuint shaderID = glCreateShader(shaderType);
    const char* src = source.c_str();
    glShaderSource(shaderID, 1, &src, NULL);
    glCompileShader(shaderID);

    GLint result;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    if (GL_FALSE == result) {
        std::string type;
        switch (shaderType) {
        case GL_COMPUTE_SHADER:
            type = "compute";
            break;
        case GL_VERTEX_SHADER:
            type = "vertex";
            break;
        case GL_TESS_CONTROL_SHADER:
            type = "tess control";
            break;
        case GL_TESS_EVALUATION_SHADER:
            type = "tess evaluation";
            break;
        case GL_GEOMETRY_SHADER:
            type = "geometry";
            break;
        case GL_FRAGMENT_SHADER:
            type = "fragment";
            break;
        }

        std::cerr << "Shader compilation failed. Shadertype: " << type << "\n";

        // Get and print the info log
        GLint logLen;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0) {
            std::string log(logLen, ' ');
            GLsizei written;
            glGetShaderInfoLog(shaderID, logLen, &written, &log[0]);
            std::cerr << "Shader log:\n" << log;
        }
        return;
    }

    _stages.push_back(shaderID);
}

void Shader::link()
{
    for (const GLuint& shader : _stages) {
        glAttachShader(_handle.id, shader);
    }
    glLinkProgram(_handle.id);

    GLint status;
    glGetProgramiv(_handle.id, GL_LINK_STATUS, &status);
    if (GL_FALSE == status) {
        std::cerr << "Failed to link shader program.\n";
        GLint logLen;
        glGetProgramiv(_handle.id, GL_INFO_LOG_LENGTH, &logLen);
        if (logLen > 0) {
            std::string log(logLen, ' ');
            GLsizei written;
            glGetProgramInfoLog(_handle.id, logLen, &written, &log[0]);
            std::cerr << "Program log:\n" << log;
        }
    }

    glValidateProgram(_handle.id);

    for (const GLuint& shader : _stages) {
        glDeleteShader(shader);
    }
}

void Shader::bind() const
{
    glUseProgram(_handle.id);
}

void Shader::unbind() const
{
    glUseProgram(0);
}

void Shader::setUniformSamplers(const std::string& name, GLsizei count, const GLint* value) const
{
    glUniform1iv(getUniformLocation(name), count, value);
}

void Shader::setUniform1f(const std::string& name, GLfloat v0) const
{
    glUniform1f(getUniformLocation(name), v0);
}

void Shader::setUniform2f(const std::string& name, GLfloat v0, GLfloat v1) const
{
    glUniform2f(getUniformLocation(name), v0, v1);
}

void Shader::setUniform3f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2) const
{
    glUniform3f(getUniformLocation(name), v0, v1, v2);
}

void Shader::setUniform4f(const std::string& name, GLfloat v0, GLfloat v1, GLfloat v2,
                          GLfloat v3) const
{
    glUniform4f(getUniformLocation(name), v0, v1, v2, v3);
}

void Shader::setUniform1i(const std::string& name, GLint v0) const
{
    glUniform1i(getUniformLocation(name), v0);
}

void Shader::setUniform2i(const std::string& name, GLint v0, GLint v1) const
{
    glUniform2i(getUniformLocation(name), v0, v1);
}

void Shader::setUniform3i(const std::string& name, GLint v0, GLint v1, GLint v2) const
{
    glUniform3i(getUniformLocation(name), v0, v1, v2);
}

void Shader::setUniform4i(const std::string& name, GLint v0, GLint v1, GLint v2, GLint v3) const
{
    glUniform4i(getUniformLocation(name), v0, v1, v2, v3);
}

void Shader::setUniform1ui(const std::string& name, GLuint v0) const
{
    glUniform1ui(getUniformLocation(name), v0);
}

void Shader::setUniform2ui(const std::string& name, GLuint v0, GLuint v1) const
{
    glUniform2ui(getUniformLocation(name), v0, v1);
}

void Shader::setUniform3ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2) const
{
    glUniform3ui(getUniformLocation(name), v0, v1, v2);
}

void Shader::setUniform4ui(const std::string& name, GLuint v0, GLuint v1, GLuint v2,
                           GLuint v3) const
{
    glUniform4ui(getUniformLocation(name), v0, v1, v2, v3);
}

void Shader::setUniform1fv(const std::string& name, const GLfloat* v) const
{
    glUniform1fv(getUniformLocation(name), 1, v);
}

void Shader::setUniform2fv(const std::string& name, const GLfloat* v) const
{
    glUniform2fv(getUniformLocation(name), 1, v);
}

void Shader::setUniform3fv(const std::string& name, const GLfloat* v) const
{
    glUniform3fv(getUniformLocation(name), 1, v);
}

void Shader::setUniform4fv(const std::string& name, const GLfloat* v) const
{
    glUniform4fv(getUniformLocation(name), 1, v);
}

void Shader::setUniform1iv(const std::string& name, const GLint* v) const
{
    glUniform1iv(getUniformLocation(name), 1, v);
}

void Shader::setUniform2iv(const std::string& name, const GLint* v) const
{
    glUniform2iv(getUniformLocation(name), 1, v);
}

void Shader::setUniform3iv(const std::string& name, const GLint* v) const
{
    glUniform3iv(getUniformLocation(name), 1, v);
}

void Shader::setUniform4iv(const std::string& name, const GLint* v) const
{
    glUniform4iv(getUniformLocation(name), 1, v);
}

void Shader::setUniform1uiv(const std::string& name, const GLuint* v) const
{
    glUniform1uiv(getUniformLocation(name), 1, v);
}

void Shader::setUniform2uiv(const std::string& name, const GLuint* v) const
{
    glUniform2uiv(getUniformLocation(name), 1, v);
}

void Shader::setUniform3uiv(const std::string& name, const GLuint* v) const
{
    glUniform3uiv(getUniformLocation(name), 1, v);
}

void Shader::setUniform4uiv(const std::string& name, const GLuint* v) const
{
    glUniform4uiv(getUniformLocation(name), 1, v);
}

void Shader::setUniformMat2f(const std::string& name, const glm::mat2& matrix,
                             const GLboolean transpose) const
{
    glUniformMatrix2fv(getUniformLocation(name), 1, transpose, &matrix[0][0]);
}

void Shader::setUniformMat3f(const std::string& name, const glm::mat3& matrix,
                             const GLboolean transpose) const
{
    glUniformMatrix3fv(getUniformLocation(name), 1, transpose, &matrix[0][0]);
}

void Shader::setUniformMat4f(const std::string& name, const glm::mat4& matrix,
                             const GLboolean transpose) const
{
    glUniformMatrix4fv(getUniformLocation(name), 1, transpose, &matrix[0][0]);
}

void Shader::setUniformMat2x3f(const std::string& name, const glm::mat2x3& matrix,
                               const GLboolean transpose) const
{
    glUniformMatrix2x3fv(getUniformLocation(name), 1, transpose, &matrix[0][0]);
}

void Shader::setUniformMat3x2f(const std::string& name, const glm::mat3x2& matrix,
                               const GLboolean transpose) const
{
    glUniformMatrix3x2fv(getUniformLocation(name), 1, transpose, &matrix[0][0]);
}

void Shader::setUniformMat2x4f(const std::string& name, const glm::mat2x4& matrix,
                               const GLboolean transpose) const
{
    glUniformMatrix2x4fv(getUniformLocation(name), 1, transpose, &matrix[0][0]);
}

void Shader::setUniformMat4x2f(const std::string& name, const glm::mat4x2& matrix,
                               const GLboolean transpose) const
{
    glUniformMatrix4x2fv(getUniformLocation(name), 1, transpose, &matrix[0][0]);
}

void Shader::setUniformMat3x4f(const std::string& name, const glm::mat3x4& matrix,
                               const GLboolean transpose) const
{
    glUniformMatrix3x4fv(getUniformLocation(name), 1, transpose, &matrix[0][0]);
}

void Shader::setUniformMat4x3f(const std::string& name, const glm::mat4x3& matrix,
                               const GLboolean transpose) const
{
    glUniformMatrix4x3fv(getUniformLocation(name), 1, transpose, &matrix[0][0]);
}

bool Shader::uniformExists(const std::string& name)
{
    return getUniformLocation(name, false) != -1;
}

std::string Shader::fileSource(const std::string& filePath) {
    std::ifstream stream(filePath);

    std::string line;
    std::stringstream ss;

    if (!stream.good()) {
        std::cout << "Failed to load shader. Path: \"" << filePath << "\"\n";
    }

    while (getline(stream, line)) {
        // File including other file
        if (line.rfind("#include", 0) == 0) {
            line.erase(0, line.find_first_of("\"<"));
            utils::trim(line);
            line.pop_back();

            // "line" is the path of the file. Example: "folder/file.txt"

            std::string thisPath
                = utils::getPath(filePath); // The path to this file. Example: "res/shaders/"
            std::string includeFile = thisPath + line;
            std::string includeText = fileSource(includeFile);
            ss << includeText << "\n";
        }
        else {
            ss << line << '\n';
        }
    }

    return ss.str();
}

GLint Shader::getUniformLocation(const std::string& name, bool warn) const
{
    if (_uniformLocations.find(name) != _uniformLocations.end())
        return _uniformLocations[name];
    GLint location = glGetUniformLocation(_handle.id, name.c_str());
    if (location == -1 && warn) {
        std::cout << "Warning: uniform " << name << " does not exist in shader: " << _handle.id
                  << std::endl;
        return -1;
    }
    _uniformLocations[name] = location;
    return location;
}
} // namespace fc::gl
