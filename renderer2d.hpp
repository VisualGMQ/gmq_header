#pragma once

#include "gogl.hpp"
#include "cgmath.hpp"
#include <optional>

namespace renderer2d {

struct Vertex final {
    cgmath::Vec2 position;
    cgmath::Vec2 texcoord;
    cgmath::Vec4 color;

    static Vertex FromPosition(const cgmath::Vec2& position) {
        return {position};
    }

    static Vertex FromPosColor(const cgmath::Vec2& position, const cgmath::Vec4& color) {
        return {position, {}, color};
    }
};

struct RectSampler final {
    gogl::Texture* texture = nullptr;
    cgmath::Rect region = {0.0, 0.0, 1.0, 1.0};
};

struct CircleSampler final {
    gogl::Texture* texture = nullptr;
    float radius = 0.5;
    cgmath::Vec2 center = {0.5, 0.5};
};

class Renderer2D {
public:
    virtual ~Renderer2D() = default;

    Renderer2D() {
        shader_ = initShader();
        whiteTexture_ = initWhiteTexture();
        vertexBuffer_ = initVertexBuffer();
        vertexBuffer_->Bind();
        indicesBuffer_ = initIndicesBuffer();
        indicesBuffer_->Bind();
        attrPtr_ = initAttrPtr(); 
        shader_->SetInt("image", 0);

        GL_CALL(glEnable(GL_BLEND));
        GL_CALL(glEnable(GL_MULTISAMPLE));
    }

    void SetViewport(const cgmath::Vec2& offset, const cgmath::Vec2& size) {
        GL_CALL(glViewport(offset.x, offset.y, size.w, size.h));
        shader_->Use();
        shader_->SetMat4("Project", cgmath::CreateOrtho(0, size.w, 0.0, size.h, -1.0, 1.0));
    }

    void SetLineWidth(int width) {
        GL_CALL(glLineWidth(width));
    }

    void SetClearColor(const cgmath::Vec4& color) {
        GL_CALL(glClearColor(color.x, color.y, color.z, color.w));
    }

    void Clear() {
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
    }

    template <typename Vertices>
    void DrawLines(const Vertices& vertices) {
        draw(gogl::PrimitiveType::LineStrip, vertices, std::array<uint32_t, 0>{});
    }

    template <typename Vertices>
    void DrawLineLoop(const Vertices& vertices,
                      const cgmath::Mat44& model = cgmath::Mat44::Identity()) {
        draw(gogl::PrimitiveType::LineLoop, vertices, std::array<uint32_t, 0>{});
    }

    void DrawLine(const cgmath::Vec2& p1, const cgmath::Vec2& p2, const cgmath::Vec4& color) {
        DrawLines(std::array<Vertex, 2>{Vertex::FromPosColor(p1, color),
                                       Vertex::FromPosColor(p2, color)});
    }

    template <typename Vertices, typename Indices>
    void DrawTriangles(Vertices vertices, const Indices& indices,
                       const cgmath::Mat44& model = cgmath::Mat44::Identity(),
                       const std::optional<RectSampler>& sampler = std::nullopt) {
        draw(gogl::PrimitiveType::Triangles, vertices, indices, model,
             sampler ? sampler->texture : nullptr);
    }

    void DrawRect(const cgmath::Rect& rect, const cgmath::Vec4& color,
                  const cgmath::Mat44& model = cgmath::Mat44::Identity()) {
        const std::array<Vertex, 4> vertices = {
            Vertex::FromPosColor({rect.x, rect.y}, color),
            Vertex::FromPosColor({rect.x + rect.w, rect.y}, color),
            Vertex::FromPosColor({rect.x + rect.w, rect.y + rect.h}, color),
            Vertex::FromPosColor({rect.x, rect.y + rect.h}, color),
        };

        DrawLineLoop(vertices, model);
    }

    void FillRect(const cgmath::Rect& rect, const cgmath::Vec4& color,
                  const cgmath::Mat44& model = cgmath::Mat44::Identity(),
                  const std::optional<RectSampler>& image = std::nullopt) {
        std::array<Vertex, 4> vertices = {
            Vertex::FromPosColor({rect.x, rect.y}, color),
            Vertex::FromPosColor({rect.x + rect.w, rect.y}, color),
            Vertex::FromPosColor({rect.x + rect.w, rect.y + rect.h}, color),
            Vertex::FromPosColor({rect.x, rect.y + rect.h}, color),
        };
        if (image) {
            const auto& region = image->region;
            vertices[0].texcoord = cgmath::Vec2{region.x, region.y};
            vertices[1].texcoord = cgmath::Vec2{region.x + region.w, region.y};
            vertices[2].texcoord = cgmath::Vec2{region.x + region.w, region.y + region.h};
            vertices[3].texcoord = cgmath::Vec2{region.x, region.y + region.h};
        }

        DrawTriangles(vertices, std::array<uint32_t, 6>{0, 1, 2, 0, 2, 3}, model, image);
    }

    void DrawArc(const cgmath::Vec2& center, float radius, float beginDeg, float endDeg, const cgmath::Vec4& color, uint32_t slice = 100) {
        std::vector<Vertex> vertices(slice);
        float step = (endDeg - beginDeg) / slice;
        for (int i = 0; i < slice; i++) {
            float radians = cgmath::Deg2Rad(beginDeg + step * i);
            vertices[i].position = cgmath::Vec2{center.x + radius * std::cos(radians),
                                                center.y + radius * std::sin(radians)};
            vertices[i].color = color;
        }
        DrawLines(vertices);
    }

    void DrawCircle(const cgmath::Vec2& center, float radius, const cgmath::Vec4& color, uint32_t slice = 100) {
        std::vector<Vertex> vertices(slice);
        float step = 2.0 * cgmath::PI / slice;
        for (int i = 0; i < slice; i++) {
            float radians = step * i;
            vertices[i].position = cgmath::Vec2{center.x + radius * std::cos(radians),
                                                center.y + radius * std::sin(radians)};
            vertices[i].color = color;
        }
        DrawLineLoop(vertices);
    }

    void FillFan(const cgmath::Vec2 &center,
                 float radius,
                 float beginDeg, float endDeg,
                 const cgmath::Vec4 &color,
                 const std::optional<CircleSampler>& sampler = std::nullopt,
                 uint32_t slice = 20) {
        std::vector<Vertex> vertices(slice);
        vertices.push_back(Vertex{center, sampler ? sampler->center : cgmath::Vec2{0.0, 0.0}, color});
        float step = (endDeg - beginDeg) / slice;
        for (int i = 0; i < slice; i++) {
            float radians = cgmath::Deg2Rad(beginDeg + step * i);
            vertices[i].position = cgmath::Vec2{center.x + radius * std::cos(radians),
                                                center.y + radius * std::sin(radians)};
            vertices[i].color = color;
            if (sampler) {
                cgmath::Vec2 size(sampler->texture->Width(), sampler->texture->Height());
                vertices[i].texcoord = sampler->center + sampler->radius * cgmath::Vec2{std::cos(radians), std::sin(radians)};
            }
        }
        draw(gogl::PrimitiveType::TriangleFan, vertices, std::array<uint32_t, 0>{}, cgmath::Mat44::Identity(),
             sampler ? sampler->texture : nullptr);
    }

    void FillCircle(const cgmath::Vec2& center,
                    float radius,
                    const cgmath::Vec4& color,
                    const std::optional<CircleSampler>& sampler = std::nullopt,
                    uint32_t slice = 20) {
        std::vector<Vertex> vertices(slice);
        vertices.push_back(Vertex{center, sampler ? sampler->center : cgmath::Vec2{0.0, 0.0}, color});
        float step = 2.0 * cgmath::PI / slice;
        for (int i = 0; i < slice; i++) {
            float radians = step * i;
            vertices[i].position = cgmath::Vec2{center.x + radius * std::cos(radians),
                                                center.y + radius * std::sin(radians)};
            vertices[i].color = color;
            if (sampler) {
                cgmath::Vec2 size(sampler->texture->Width(), sampler->texture->Height());
                vertices[i].texcoord = sampler->center + sampler->radius * cgmath::Vec2{std::cos(radians), std::sin(radians)};
            }
        }
        draw(gogl::PrimitiveType::TriangleFan,
             vertices,
             std::array<uint32_t, 0>{},
             cgmath::Mat44::Identity(),
             sampler ? sampler->texture : nullptr);
    }

private:
    std::unique_ptr<gogl::Shader> shader_;
    std::unique_ptr<gogl::Texture> whiteTexture_;
    std::unique_ptr<gogl::Buffer> vertexBuffer_;
    std::unique_ptr<gogl::Buffer> indicesBuffer_;
    std::unique_ptr<gogl::AttributePointer> attrPtr_;

    template <typename Vertices, typename Indices>
    void draw(gogl::PrimitiveType primitive, Vertices vertices, const Indices& indices,
                const cgmath::Mat44& model = cgmath::Mat44::Identity(),
                gogl::Texture* texture = nullptr) {
        static_assert(std::is_same_v<Vertices::value_type, Vertex>);
        static_assert(std::is_same_v<Indices::value_type, uint32_t>);
        if (indices.empty()) {
            indicesBuffer_->Unbind();
        } else {
            indicesBuffer_->Bind();
            indicesBuffer_->SetData((void*)indices.data(), sizeof(uint32_t) * indices.size());
        }
        if (!texture) {
            whiteTexture_->Bind();
        } else {
            texture->Bind();
            int w = texture->Width();
            int h = texture->Height();
        }
        vertexBuffer_->Bind();
        vertexBuffer_->SetData((void*)vertices.data(), sizeof(Vertex) * vertices.size());
        attrPtr_->Bind();
        shader_->Use();
        shader_->SetMat4("Model", model);

        if (indices.empty()) {
            shader_->DrawArray(primitive, 0, vertices.size());
        } else {
            shader_->DrawElements(primitive, indices.size(), GL_UNSIGNED_INT, 0);
        }

    }

    std::unique_ptr<gogl::Shader> initShader() {
        gogl::ShaderModule vertexModel(gogl::ShaderModule::Type::Vertex, R"(
            #version 430 core
            layout (location = 0) in vec2 aPos;
            layout (location = 1) in vec2 aTexCoord;
            layout (location = 2) in vec4 aColor;

            out vec2 TexCoord;
            out vec4 Color;

            uniform mat4 Project;
            uniform mat4 Model;

            void main() {
                TexCoord = aTexCoord;
                Color = aColor;
                gl_Position = Project * Model * vec4(aPos, 0.0, 1.0);
            }
        )");

        gogl::ShaderModule fragmentModel(gogl::ShaderModule::Type::Fragment, R"(
            #version 430 core

            in vec2 TexCoord;
            in vec4 Color;

            uniform sampler2D image;

            out vec4 FragColor;

            void main() {
                FragColor = texture(image, TexCoord) * Color;
            }
        )");

        return std::make_unique<gogl::Shader>(vertexModel, fragmentModel);
    }

    std::unique_ptr<gogl::Texture> initWhiteTexture() {
        uint32_t pixels[] = {0xFFFFFFFF};
        return std::make_unique<gogl::Texture>(
            gogl::Texture::Type::Dimension2, pixels, 1, 1,
            gogl::Sampler::CreateLinearRepeat(), gogl::Format::RGBA,
            gogl::Format::RGBA);
    }

    std::unique_ptr<gogl::Buffer> initVertexBuffer() {
        return std::make_unique<gogl::Buffer>(gogl::BufferType::Array);
    }

    std::unique_ptr<gogl::Buffer> initIndicesBuffer() {
        return std::make_unique<gogl::Buffer>(gogl::BufferType::Element);
    }

    std::unique_ptr<gogl::AttributePointer> initAttrPtr() {
        return std::make_unique<gogl::AttributePointer>(
            gogl::BufferLayout::CreateFromTypes({gogl::Attribute::Type::Vec2,
                                                 gogl::Attribute::Type::Vec2,
                                                 gogl::Attribute::Type::Vec4}));
    }
};

}