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

#define GL_RET_CALL(expr) [&]() { \
    GLClearError(); \
    auto value = expr; \
    GLenum e = glGetError(); \
    if (e != GL_NO_ERROR) { \
        LOGE("[GL]: ", GLError2Str(e)); \
    } \
    return value; \
}()

/************ Shader *****************/

class Shader;

class ShaderModule final {
public:
    friend class Shader;

    enum class Type {
        Vertex,
        Fragment,
        Compute,
    };

    static auto CreateVertexShader(const std::string& code) {
        return ShaderModule(Type::Vertex, code);
    }

    static auto CreateFragmentShader(const std::string& code) {
        return ShaderModule(Type::Fragment, code);
    }

    static auto CreateComputeShader(const std::string& code) {
        return ShaderModule(Type::Compute, code);
    }

    ShaderModule(const ShaderModule&) = delete;
    ShaderModule& operator=(const ShaderModule&) = delete;

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
            case ShaderModule::Type::Compute: return GL_COMPUTE_SHADER;
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

enum class PrimitiveType {
    Points,
    LineStrip,
    LineLoop,
    Lines,
    LineStripAdjacency,
    LinesAdjacency,
    TriangleStrip,
    TriangleFan,
    Triangles,
    TriangleStripAdjacency,
    TrianglesAdjacency,
    Patches,
};

inline GLenum Primitive2GL(PrimitiveType type) {
    switch (type) {
        case PrimitiveType::Points : return GL_POINTS;
        case PrimitiveType::LineStrip : return GL_LINE_STRIP;
        case PrimitiveType::LineLoop : return GL_LINE_LOOP;
        case PrimitiveType::Lines : return GL_LINES;
        case PrimitiveType::LineStripAdjacency : return GL_LINE_STRIP_ADJACENCY;
        case PrimitiveType::LinesAdjacency : return GL_LINES_ADJACENCY;
        case PrimitiveType::TriangleStrip : return GL_TRIANGLE_STRIP;
        case PrimitiveType::TriangleFan : return GL_TRIANGLE_FAN;
        case PrimitiveType::Triangles : return GL_TRIANGLES;
        case PrimitiveType::TriangleStripAdjacency : return GL_TRIANGLE_STRIP_ADJACENCY;
        case PrimitiveType::TrianglesAdjacency : return GL_TRIANGLES_ADJACENCY;
        case PrimitiveType::Patches : return GL_PATCHES;
    }
}

enum BarrierType {
    AtomicCounter,
    Framebuffer,
    ShaderImageAccess,
    ShaderStorage,
    TextureFetch,
    Uniform,
};

inline GLenum Barrier2GL(BarrierType type) {
    switch (type) {
        case BarrierType::AtomicCounter: return GL_ATOMIC_COUNTER_BARRIER_BIT;
        case BarrierType::Framebuffer: return GL_FRAMEBUFFER_BARRIER_BIT;
        case BarrierType::ShaderImageAccess: return GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
        case BarrierType::ShaderStorage: return GL_SHADER_STORAGE_BARRIER_BIT;
        case BarrierType::TextureFetch: return GL_TEXTURE_FETCH_BARRIER_BIT;
        case BarrierType::Uniform: return GL_UNIFORM_BARRIER_BIT;
    }
}

class Shader final {
public:
    static Shader CreateGraphics(const ShaderModule& vertex, const ShaderModule& fragment) {
        return Shader(vertex, fragment);
    }

    static Shader CreateCompute(const ShaderModule& compute) {
        return Shader(compute);
    }

    Shader(const ShaderModule& compute): type_(Type::Compute) {
        id_ = GL_RET_CALL(glCreateProgram());

        GL_CALL(glAttachShader(id_, compute.id_));
        GL_CALL(glLinkProgram(id_));

        int success;
        char infoLog[1024];
        GL_CALL(glGetProgramiv(id_, GL_LINK_STATUS, &success));
        if(!success) {
            glGetProgramInfoLog(id_, 1024, NULL, infoLog);
            LOGF("[GL]: shader link failed:\r\n", infoLog);
        }
    }

    Shader(const ShaderModule& vertex, const ShaderModule& fragment): type_(Type::Graphics) {
        id_ = GL_RET_CALL(glCreateProgram());

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

    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;

    ~Shader() {
        GL_CALL(glDeleteProgram(id_));
    }

    void Use() const { GL_CALL(glUseProgram(id_)); }
    void Unuse() const { GL_CALL(glUseProgram(0)); }

    void SetMat4(std::string_view name, const cgmath::Mat44& m) {
        Use();
        auto loc = GL_RET_CALL(glGetUniformLocation(id_, name.data()));
        if (loc == -1) {
            LOGE("[GL]: don't has uniform ", name);
        } else {
            GL_CALL(glUniformMatrix4fv(loc, 1, GL_FALSE, m.data));
        }
    }

    void SetVec3(std::string_view name, const cgmath::Vec3& v) {
        Use();
        auto loc = GL_RET_CALL(glGetUniformLocation(id_, name.data()));
        if (loc == -1) {
            LOGE("[GL]: don't has uniform ", name);
        } else {
            GL_CALL(glUniform3f(loc, v.x, v.y, v.z));
        }
    }

    void SetInt(std::string_view name, int value) {
        Use();
        auto loc = GL_RET_CALL(glGetUniformLocation(id_, name.data()));
        if (loc == -1) {
            LOGE("[GL]: don't has uniform ", name);
        } else {
            GL_CALL(glUniform1i(loc, value));
        }
    }

    void DrawArray(PrimitiveType type, uint64_t first, uint64_t count) {
        if (type_ != Type::Graphics) {
            LOGE("[Shader]: your shader isn't graphics shader");
        } else {
            GL_CALL(glDrawArrays(Primitive2GL(type), first, count));
        }
    }

    void DispatchCompute(uint64_t x, uint64_t y, uint64_t z) {
        if (type_ != Type::Compute) {
            LOGE("[Shader]: your shader isn't graphics shader");
        } else {
            GL_CALL(glDispatchCompute(x, y, z));
        }
    }

    void WaitMemoryBarrier(BarrierType type) {
        GL_CALL(glMemoryBarrier(Barrier2GL(type)));
    }

private:
    GLuint id_ = 0;

    enum Type {
        Graphics,
        Compute,
    } type_;
};

/************ Buffer *****************/
enum class BufferType {
    Array,
    Element,
    ShaderStorage,
    AtomicCounter,
    CopyRead,
    CopyWrite,
    DispatchIndirect,
    DrawIndirect,
    PixelPack,
    PixelUnpack,
    QueryBuffer,
    TextureBuffer,
    TransformFeedback,
    UniformBuffer,
};

inline GLenum BufferType2GL(BufferType type) {
    switch (type) {
        case BufferType::Array: return GL_ARRAY_BUFFER;
        case BufferType::Element: return GL_ELEMENT_ARRAY_BUFFER;
        case BufferType::ShaderStorage: return GL_SHADER_STORAGE_BUFFER;
        case BufferType::AtomicCounter: return GL_ATOMIC_COUNTER_BUFFER;
        case BufferType::CopyRead: return GL_COPY_READ_BUFFER;
        case BufferType::CopyWrite: return GL_COPY_WRITE_BUFFER;
        case BufferType::DispatchIndirect: return GL_DISPATCH_INDIRECT_BUFFER;
        case BufferType::DrawIndirect: return GL_DRAW_INDIRECT_BUFFER;
        case BufferType::PixelPack: return GL_PIXEL_PACK_BUFFER;
        case BufferType::PixelUnpack: return GL_PIXEL_UNPACK_BUFFER;
        case BufferType::QueryBuffer: return GL_QUERY_BUFFER;
        case BufferType::TextureBuffer: return GL_TEXTURE_BUFFER;
        case BufferType::TransformFeedback: return GL_TRANSFORM_FEEDBACK_BUFFER;
        case BufferType::UniformBuffer: return GL_UNIFORM_BUFFER;
    }
}

inline std::string_view BufferType2Str(BufferType type) {
    switch (type) {
        case BufferType::Array: return "Array";
        case BufferType::Element: return "Element";
        case BufferType::ShaderStorage: return "ShaderStorage";
        case BufferType::AtomicCounter: return "AtomicCounter";
        case BufferType::CopyRead: return "CopyRead";
        case BufferType::CopyWrite: return "CopyWrite";
        case BufferType::DispatchIndirect: return "DispatchIndirect";
        case BufferType::DrawIndirect: return "DrawIndirect";
        case BufferType::PixelPack: return "PixelPack";
        case BufferType::PixelUnpack: return "PixelUnpack";
        case BufferType::QueryBuffer: return "QueryBuffer";
        case BufferType::TextureBuffer: return "TextureBuffer";
        case BufferType::TransformFeedback: return "TransformFeedback";
        case BufferType::UniformBuffer: return "UniformBuffer";
    }
    return "Unknown";
}

class Buffer final {
public:
    enum Access {
        ReadOnly,
        WriteOnly,
        ReadWrite,

        InvalidRange,
        InvalidBuffer,
        FlushExplicit,
    };

    struct MapGuard final {
        MapGuard(Buffer& buffer, void* ptr): buffer_(buffer), ptr(ptr) { }
        ~MapGuard() {
            Unmap();
        }
        void Unmap() {
            if (!unmapped_) {
                buffer_.Bind();
                GL_CALL(glUnmapBuffer(BufferType2GL(buffer_.type_)));
                unmapped_ = true;
            }
        }

        MapGuard(const MapGuard&) = delete;
        MapGuard& operator=(const MapGuard&) = delete;

        void* ptr;

    private:
        bool unmapped_ = false;
        Buffer& buffer_;
    };

    Buffer(BufferType type): type_(type) {
        GL_CALL(glGenBuffers(1, &id_));
    }

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    ~Buffer() {
        GL_CALL(glDeleteBuffers(1, &id_));
    }

    void SetData(void* datas, size_t size) {
        Bind();
        GL_CALL(glBufferData(BufferType2GL(type_), size, datas, GL_STATIC_DRAW));
    }

    void GetData(void* buf) const {
        Bind();
        size_t size = Size();
        GL_CALL(glGetBufferSubData(BufferType2GL(type_), 0, size, buf));
    }

    void GetSubData(void* buf, size_t offset, size_t len) const {
        Bind();
        GL_CALL(glGetBufferSubData(BufferType2GL(type_), offset, len, buf));
    }

    void ExtendSize(size_t size) {
        SetData(nullptr, size);
    }

    MapGuard Map(Access access) {
        Bind();
        return MapGuard(*this, GL_RET_CALL(glMapBuffer(BufferType2GL(type_), access2gl(access))));
    }

    MapGuard MapRange(size_t offset, size_t len, Access access) {
        Bind();
        return MapGuard(*this, GL_RET_CALL(glMapBufferRange(BufferType2GL(type_), offset, len, access2gl(access))));
    }

    void Flush(size_t offset, size_t len) {
        Bind();
        GL_CALL(glFlushMappedBufferRange(BufferType2GL(type_), offset, len));
    }

    void Bind() const {
        GL_CALL(glBindBuffer(BufferType2GL(type_), id_));
    }

    void Unbind() const {
        GL_CALL(glBindBuffer(BufferType2GL(type_), 0));
    }

    void Bind2Base(uint8_t slot) const {
        if (type_ != BufferType::AtomicCounter &&
            type_ != BufferType::TransformFeedback &&
            type_ != BufferType::UniformBuffer &&
            type_ != BufferType::ShaderStorage) {
            LOGW("the buffer type don't support bind to base");
        } else {
            GL_CALL(glBindBufferBase(BufferType2GL(type_), slot, id_));
        }
    }

    int Size() const {
        GLint size = 0;
        Bind();
        GL_CALL(glGetBufferParameteriv(BufferType2GL(type_), GL_BUFFER_SIZE, &size));
        return size;
    }

private:
    GLuint id_;
    BufferType type_;

    GLenum access2gl(Access access) const {
        switch (access) {
            case Access::ReadOnly: return GL_READ_ONLY;
            case Access::WriteOnly: return GL_WRITE_ONLY;
            case Access::ReadWrite: return GL_READ_WRITE;
            case Access::InvalidBuffer: return GL_MAP_INVALIDATE_BUFFER_BIT;
            case Access::InvalidRange: return GL_MAP_INVALIDATE_RANGE_BIT;
            case Access::FlushExplicit: return GL_MAP_FLUSH_EXPLICIT_BIT;
        }
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

    static Sampler CreateLinearRepeat() {
        Sampler sampler;
        sampler.mipmap = false;
        sampler.filter.mag = TextureFilerType::Linear;
        sampler.filter.min = TextureFilerType::Linear;
        sampler.wrapper.s = TextureWrapperType::Repeat;
        sampler.wrapper.r = TextureWrapperType::Repeat;
        sampler.wrapper.t = TextureWrapperType::Repeat;
        return sampler;
    }

    static Sampler CreateNearestRepeat() {
        Sampler sampler;
        sampler.mipmap = false;
        sampler.filter.mag = TextureFilerType::Nearest;
        sampler.filter.min = TextureFilerType::Nearest;
        sampler.wrapper.s = TextureWrapperType::Repeat;
        sampler.wrapper.r = TextureWrapperType::Repeat;
        sampler.wrapper.t = TextureWrapperType::Repeat;
        return sampler;
    }
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

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;

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


inline uint8_t GetAttributeTypeCount(Attribute::Type type) {
    switch (type) {
        case Attribute::Type::Vec2: return 2;
        case Attribute::Type::Vec3: return 3;
        case Attribute::Type::Vec4: return 4;
        case Attribute::Type::Mat2: return 2 * 2;
        case Attribute::Type::Mat3: return 3 * 3;
        case Attribute::Type::Mat4: return 4 * 4;
    }
    LOGW("[Attribute Type]: Unknown attribute type");
    return 0;
}

inline uint8_t GetAttributeTypeSize(Attribute::Type type) {
    uint8_t typeCount = GetAttributeTypeCount(type);
    switch (type) {
        case Attribute::Type::Vec2: return typeCount * 4;
        case Attribute::Type::Vec3: return typeCount * 4;
        case Attribute::Type::Vec4: return typeCount * 4;
        case Attribute::Type::Mat2: return 4 * typeCount;
        case Attribute::Type::Mat3: return 4 * typeCount;
        case Attribute::Type::Mat4: return 4 * typeCount;
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

        return BufferLayout(std::move(attrs), offset);
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

        return BufferLayout(std::move(attrs), offset);
    }

    static auto CreateFromAttrs(const std::initializer_list<Attribute>& attrs) {
        size_t stride = 0;
        for (auto& attr : attrs) {
            stride += GetAttributeTypeSize(attr.type);
        }
        return BufferLayout(attrs, stride);
    }

    BufferLayout(std::vector<Attribute>&& attrs, size_t stride): attributes_(std::move(attrs)), stride_(stride) {}
    BufferLayout(const std::vector<Attribute>& attrs, size_t stride): attributes_(attrs), stride_(stride) {}

    auto& Attributes() const { return attributes_; }
    auto Stride() const { return stride_; }

private:
    std::vector<Attribute> attributes_;
    size_t stride_ = 0;
};

class AttributePointer final {
public:
    AttributePointer(const BufferLayout& layout): layout_(layout) {
        GL_CALL(glGenVertexArrays(1, &id_));
        bindAttributes(layout_);
    }

    AttributePointer(BufferLayout&& layout): layout_(std::move(layout)) {
        GL_CALL(glGenVertexArrays(1, &id_));
        bindAttributes(layout_);
    }

    AttributePointer(const AttributePointer&) = delete;
    AttributePointer& operator=(const AttributePointer&) = delete;

    ~AttributePointer() {
        GL_CALL(glDeleteVertexArrays(1, &id_));
    }

    void Bind() const {
        GL_CALL(glBindVertexArray(id_));
    }

    void Unbind() const {
        GL_CALL(glBindVertexArray(0));
    }

private:
    BufferLayout layout_;
    GLuint id_;

    void bindAttributes(const BufferLayout& layout) {
        Bind();
        for (auto& attr : layout.Attributes()) {
            GL_CALL(glVertexAttribPointer(attr.location,
                                          GetAttributeTypeCount(attr.type),
                                          GL_FLOAT,
                                          GL_FALSE,
                                          layout.Stride(),
                                          (void*)attr.offset));
            GL_CALL(glEnableVertexAttribArray(attr.location));
        }
    }

};

/************ physic device *****************/
class PhysicDevice final {
public:
    std::array<int, 3> GetComputeWorkGroupMaxSize() const {
        std::array<int, 3> work_grp_cnt;
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);
        return work_grp_cnt;
    }

    int GetComputeGroupInvocationMaxCount() const {
        int work_grp_inv;
        GL_CALL(glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv));
        return work_grp_inv;
    }
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
        return result.first->second;
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
class AttrPointerManager: public ResManager<AttributePointer> {};
