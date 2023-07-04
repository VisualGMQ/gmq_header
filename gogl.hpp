#pragma once

#include "glad/glad.h"
#include "log.hpp"
#include "cgmath.hpp"
#include <unordered_map>
#include <optional>

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

    static auto CreateVertexShader(const std::string& code) {
        return ShaderModule(Type::Vertex, code);
    }

    static auto CreateFragmentShader(const std::string& code) {
        return ShaderModule(Type::Fragment, code);
    }

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

/************ texture *****************/
enum class TextureWrapperType {
    Repeat = GL_REPEAT,
    MirroredRepeat = GL_MIRRORED_REPEAT,
    ClampToEdge = GL_CLAMP_TO_EDGE,
    ClampToBorder = GL_CLAMP_TO_BORDER,
};

enum class TextureFilerType {
    Nearest = GL_NEAREST,
    Linear = GL_LINEAR,

    LinearMipmapLinear = GL_LINEAR_MIPMAP_LINEAR,
    LinearMipmapNearest = GL_LINEAR_MIPMAP_NEAREST,
    NearestMipmapLinear = GL_NEAREST_MIPMAP_LINEAR,
    NearestMipmapNearest = GL_NEAREST_MIPMAP_NEAREST,
};

struct Sampler final {
    struct {
        std::optional<TextureWrapperType> s, r, t;
        float borderColor[4] = {1, 1, 1, 1};

        bool NeedBorderColor() const {
            return s == TextureWrapperType::ClampToBorder ||
                    r == TextureWrapperType::ClampToBorder ||
                    t == TextureWrapperType::ClampToBorder;
        }
    } wrapper;
    struct {
        std::optional<TextureFilerType> min, mag;
    } filter;
    bool mipmap;
};

enum class Format {
    RED = GL_RED,
    RG = GL_RG,
    RGB = GL_RGB,
    BGR = GL_BGR,
    RGBA = GL_RGBA,
    BGRA = GL_BGRA,
    RED_INTEGER = GL_RED_INTEGER,
    RG_INTEGER = GL_RG_INTEGER,
    RGB_INTEGER = GL_RGB_INTEGER,
    BGR_INTEGER = GL_BGR_INTEGER,
    RGBA_INTEGER = GL_RGBA_INTEGER,
    BGRA_INTEGER = GL_BGRA_INTEGER,
    STENCIL_INDEX = GL_STENCIL_INDEX,
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL = GL_DEPTH_STENCIL
};

class Texture final {
public:
    enum class Type {
        Dimension2 = GL_TEXTURE_2D,
        // TODO: do other type support later
        // Dimension3 = GL_TEXTURE_3D,
    };

    Texture(Type type, void* pixels, int w, int h, const Sampler& sampler, Format out, Format inner): type_(type) {
        GL_CALL(glGenTextures(1, &id_));
        Bind();
        if (sampler.wrapper.s) {
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, static_cast<GLint>(sampler.wrapper.s.value())));   
        }
        if (sampler.wrapper.r) {
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, static_cast<GLint>(sampler.wrapper.r.value())));   
        }
        if (sampler.wrapper.t) {
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, static_cast<GLint>(sampler.wrapper.t.value())));   
        }
        if (sampler.wrapper.NeedBorderColor()) {
            GL_CALL(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, sampler.wrapper.borderColor));
        }
        if (sampler.filter.min) {
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, static_cast<GLint>(sampler.filter.min.value())));
        }
        if (sampler.filter.mag) {
            GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, static_cast<GLint>(sampler.filter.mag.value())));
        }
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0,
                     static_cast<GLint>(inner),
                     w, h,
                     sampler.wrapper.NeedBorderColor(),
                     static_cast<GLint>(inner),
                     GL_UNSIGNED_BYTE,
                     pixels));
        if (sampler.mipmap) {
            GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
        }
    }

    void Bind(int slot = 0) const {
        GL_CALL(glActiveTexture(GL_TEXTURE0 + slot));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, id_));
    }

    void Unbind() const {
        GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    }

    ~Texture() {
        GL_CALL(glDeleteTextures(1, &id_));
    }

private:
    GLuint id_;
    Type type_;
};

/************ buffer layout *****************/
struct Attribute final {
    enum class Type {
        Vec2,
        Vec3,
        Vec4,
        Mat2, 
        Mat3,
        Mat4
    } type;
    int location;
    size_t offset;

    Attribute(Type type, int location, size_t offset) : type(type), location(location), offset(offset) {}
    Attribute(Type type, int location) : type(type), location(location), offset(0) {}
};

inline size_t GetAttributeTypeSize(Attribute::Type type) {
    switch (type) {
        case Attribute::Type::Vec2: return 2 * 4;
        case Attribute::Type::Vec3: return 3 * 4;
        case Attribute::Type::Vec4: return 4 * 4;
        case Attribute::Type::Mat2: return 4 * 2 * 2;
        case Attribute::Type::Mat3: return 4 * 3 * 3;
        case Attribute::Type::Mat4: return 4 * 4 * 4;
    }
    LOGW("[Attribute Type]: Unknown attribute type");
    return 0;
}

class BufferLayout final {
public:
    static auto CreateFromTypes(const std::initializer_list<Attribute::Type>& types) {
        std::vector<Attribute> attrs;
        int i = 0;
        size_t offset = 0;
        for (auto type : types) {
            size_t size = GetAttributeTypeSize(type);
            attrs.push_back(Attribute(type, i++, offset));
            offset += size;
        }

        return BufferLayout(std::move(attrs));
    }

    static auto CreateFromUnoffsetAttrs(const std::initializer_list<Attribute>& attrs) {
        std::vector<Attribute> attributes;
        int i = 0;
        size_t offset = 0;
        for (auto attr : attrs) {
            size_t size = GetAttributeTypeSize(attr.type);
            attributes.push_back(Attribute(attr.type, i++, offset));
            offset += size;
        }

        return BufferLayout(std::move(attrs));
    }

    static auto CreateFromAttrs(const std::initializer_list<Attribute>& attrs) {
        return BufferLayout(attrs);
    }

    BufferLayout(std::vector<Attribute>&& attrs): attributes_(std::move(attrs)) {}
    BufferLayout(const std::vector<Attribute>& attrs): attributes_(attrs) {}

private:
    std::vector<Attribute> attributes_;
};

/************ abstract resource manager *****************/

template <typename T>
class ResManager {
public:
    template <typename... Args>
    T& Create(const std::string& name, Args&&... args) {
        auto result = resources_.emplace(std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(std::forward<Args>(args)...)
        );
        if (!result.second) {
            LOGE("[ResManager]: emplace %s failed! Maybe C++ inner error", name.c_str());
        }
        return *result.first;
    }

    void Destory(const std::string& name) {
        resources_.erase(name);
    }

    T* Find(const std::string& name) {
        if (auto it = resources_.find(name); it != resources_.end()) {
            return it.second;
        }
        return nullptr;
    }

    virtual ~ResManager() { resources_.clear(); }

private:
    std::unordered_map<std::string, T> resources_;
};

class ShaderManager: public ResManager<Shader> {};
class BufferManager: public ResManager<Buffer> {};
class TextureManager: public ResManager<Texture> {};
