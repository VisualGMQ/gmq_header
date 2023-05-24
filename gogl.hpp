#pragma once

#include "glad/glad.h"
#include "log.hpp"
#include "cgmath.hpp"

/******** some macro to make opengl call safer *********/

#define _GL_MAX_ERROR 1024

inline void GLClearError() {
    static unsigned int count = 0;
    count = 0;
    GLenum err = glGetError();
    while (err != GL_NO_ERROR && count < _GL_MAX_ERROR) {
        err = glGetError();
        count ++;
    }
}

inline const char* GLError2Str(GLenum error) {
    #define CASE(x) case x: return #x;
    switch (error) {
        CASE(GL_INVALID_ENUM)
        CASE(GL_INVALID_VALUE)
        CASE(GL_INVALID_OPERATION)
        CASE(GL_INVALID_FRAMEBUFFER_OPERATION)
        CASE(GL_OUT_OF_MEMORY)
#ifdef GL_STACK_UNDERFLOW
        CASE(GL_STACK_UNDERFLOW)
#endif

#ifdef GL_STACK_OVERFLOW
        CASE(GL_STACK_OVERFLOW)
#endif
        default: return "GL_UNKNOWN_ERROR";
    } 
    #undef CASE
}

#define GL_CALL(expr) do { \
    GLClearError(); \
    expr; \
    GLenum e = glGetError(); \
    if (e != GL_NO_ERROR) { \
        LOGE("[GL]: ", GLError2Str(e)); \
    } \
} while(0)

/************ Shader *****************/

class Shader;

class ShaderModule final {
public:
    friend class Shader;

    enum class Type {
        Vertex,
        Fragment,
    };

    ShaderModule(Type type, const std::string& code): type_(type) {
        id_ = glCreateShader(type2gl(type));
        const char* source = code.c_str();
        GL_CALL(glShaderSource(id_, 1, &source, nullptr));
        GL_CALL(glCompileShader(id_));

        int success;
        char infoLog[1024];
        GL_CALL(glGetShaderiv(id_, GL_COMPILE_STATUS, &success));
        if(!success) {
            GL_CALL(glGetShaderInfoLog(id_, 1024, NULL, infoLog));
            LOGF("[GL] :", type2gl(type), " shader compile failed:\r\n", infoLog);
        }
    }
    ~ShaderModule() {
        GL_CALL(glDeleteShader(id_));
    }

private:
    GLuint id_ = 0;
    Type type_;

    GLenum type2gl(ShaderModule::Type type) {
        switch (type) {
            case ShaderModule::Type::Vertex: return GL_VERTEX_SHADER;
            case ShaderModule::Type::Fragment: return GL_FRAGMENT_SHADER;
        }
    }

    std::string_view type2str(ShaderModule::Type type) {
        switch (type) {
            case ShaderModule::Type::Vertex: return "Vertex";
            case ShaderModule::Type::Fragment: return "Fragment";
        }
        return "Unkown";
    }

};

class Shader final {
public:
    Shader(const ShaderModule& vertex, const ShaderModule& fragment) {
        id_ = glCreateProgram();

        GL_CALL(glAttachShader(id_, vertex.id_));
        GL_CALL(glAttachShader(id_, fragment.id_));
        GL_CALL(glLinkProgram(id_));

        int success;
        char infoLog[1024];
        GL_CALL(glGetProgramiv(id_, GL_LINK_STATUS, &success));
        if(!success) {
            glGetProgramInfoLog(id_, 1024, NULL, infoLog);
            LOGF("[GL]: shader link failed:\r\n", infoLog);
        }
    }
    ~Shader() {
        GL_CALL(glDeleteProgram(id_));
    }

    void Use() { GL_CALL(glUseProgram(id_)); }
    void Unuse() { GL_CALL(glUseProgram(0)); }

    void SetMat4(std::string_view name, const cgmath::Mat44& m) {
        auto loc = glGetUniformLocation(id_, name.data());
        if (loc == -1) {
            LOGE("[GL]: don't has uniform ", name);
        } else {
            GL_CALL(glUniformMatrix4fv(loc, 1, GL_FALSE, m.data));
        }
    }

    void SetVec3(std::string_view name, const cgmath::Vec3& v) {
        auto loc = glGetUniformLocation(id_, name.data());
        if (loc == -1) {
            LOGE("[GL]: don't has uniform ", name);
        } else {
            GL_CALL(glUniform3f(loc, v.x, v.y, v.z));
        }
    }

private:
    GLuint id_ = 0;
};

/************ Buffer *****************/
class Buffer final {
public:
    enum Type {
        Array,
        Element,
    };

    Buffer(Type type) {
        GL_CALL(glGenBuffers(1, &id_));
    }
    ~Buffer() {
        GL_CALL(glDeleteBuffers(1, &id_));
    }

    void SetData(void* datas, size_t size) {
        Bind();
        GL_CALL(glBufferData(type2gl(type_), size, datas, GL_STATIC_DRAW));
    }

    void Bind() {
        glBindBuffer(type2gl(type_), id_);
    }

    void Unbind() {
        glBindBuffer(type2gl(type_), 0);
    }

private:
    GLuint id_;
    Type type_;

    GLenum type2gl(Buffer::Type type) {
        switch (type) {
            case Buffer::Type::Array: return GL_ARRAY_BUFFER;
            case Buffer::Type::Element: return GL_ELEMENT_ARRAY_BUFFER;
        }
    }

    std::string_view type2str(Buffer::Type type) {
        switch (type) {
            case Buffer::Type::Array: return "Array";
            case Buffer::Type::Element: return "Element";
        }
        return "Unknown";
    }
};