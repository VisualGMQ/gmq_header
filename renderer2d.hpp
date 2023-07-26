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

struct Image final {
    gogl::Texture* texture = nullptr;
    std::optional<cgmath::Rect> region;
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

    void SetClearColor(const cgmath::Vec4& color) {
        GL_CALL(glClearColor(color.x, color.y, color.z, color.w));
    }

    void Clear() {
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
    }

    template <typename Vertices>
    void DrawLines(const Vertices& vertices, const gogl::Texture& texture = gogl::Texture::Null()) {
        vertexBuffer_->Bind();
        indicesBuffer_->Unbind();
        vertexBuffer_->SetData((const void*)vertices.data(), sizeof(Vertex) * vertices.size());
        attrPtr_->Bind();
        shader_->Use();
        if (texture == gogl::Texture::Null()) {
            whiteTexture_->Bind();
        } else {
            texture.Bind();
        }
        shader_->SetMat4("Model", cgmath::Mat44::Identity());
        shader_->DrawArray(gogl::PrimitiveType::LineStrip, 0, vertices.size());
    }

    template <typename Vertices>
    void DrawLineLoop(const Vertices& vertices,
                      const cgmath::Mat44& model = cgmath::Mat44::Identity(),
                      const gogl::Texture& texture = gogl::Texture::Null()) {
        vertexBuffer_->Bind();
        indicesBuffer_->Unbind();
        vertexBuffer_->SetData((void*)vertices.data(), sizeof(Vertex) * vertices.size());
        attrPtr_->Bind();
        shader_->Use();
        if (texture == gogl::Texture::Null()) {
            whiteTexture_->Bind();
        } else {
            texture.Bind();
        }
        shader_->SetMat4("Model", model);
        shader_->DrawArray(gogl::PrimitiveType::LineLoop, 0, vertices.size());
    }

    void DrawLine(const cgmath::Vec2& p1, const cgmath::Vec2& p2, const cgmath::Vec4& color) {
        DrawLines(std::array<Vertex, 2>{Vertex::FromPosColor(p1, color),
                                       Vertex::FromPosColor(p2, color)});
    }

    template <typename Vertices, typename Indices>
    void DrawTriangles(Vertices vertices, const Indices& indices,
                       const cgmath::Mat44& model = cgmath::Mat44::Identity(),
                       const std::optional<Image>& image = std::nullopt) {
        static_assert(std::is_same_v<Vertices::value_type, Vertex>);
        static_assert(std::is_same_v<Indices::value_type, uint32_t>);
        if (indices.empty()) {
            indicesBuffer_->Unbind();
        } else {
            indicesBuffer_->Bind();
            indicesBuffer_->SetData((void*)indices.data(), sizeof(uint32_t) * indices.size());
        }
        if (!image.has_value()) {
            whiteTexture_->Bind();
        } else {
            image->texture->Bind();
            int w = image->texture->Width();
            int h = image->texture->Height();
            const auto& region = image->region;
            vertices[0].texcoord = region.has_value() ? cgmath::Vec2{region->x / w, region->y / h} : cgmath::Vec2(0.0, 0.0);
            vertices[1].texcoord = region.has_value() ? cgmath::Vec2{(region->x + region->w) / w, region->y / h} : cgmath::Vec2(1.0, 0.0);
            vertices[2].texcoord = region.has_value() ? cgmath::Vec2{(region->x + region->w) / w, (region->y + region->h) / h} : cgmath::Vec2(1.0, 1.0);
            vertices[3].texcoord = region.has_value() ? cgmath::Vec2{region->x / w, (region->y + region->h) / h} : cgmath::Vec2(0.0, 1.0);
        }
        vertexBuffer_->Bind();
        vertexBuffer_->SetData((void*)vertices.data(), sizeof(Vertex) * vertices.size());
        attrPtr_->Bind();
        shader_->Use();
        shader_->SetMat4("Model", model);

        if (indices.empty()) {
            shader_->DrawArray(gogl::PrimitiveType::Triangles, 0, vertices.size());
        } else {
            shader_->DrawElements(gogl::PrimitiveType::Triangles, indices.size(), GL_UNSIGNED_INT, 0);
        }
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
                  const std::optional<Image>& image = std::nullopt) {
        const std::array<Vertex, 4> vertices = {
            Vertex::FromPosColor({rect.x, rect.y}, color),
            Vertex::FromPosColor({rect.x + rect.w, rect.y}, color),
            Vertex::FromPosColor({rect.x + rect.w, rect.y + rect.h}, color),
            Vertex::FromPosColor({rect.x, rect.y + rect.h}, color),
        };
        DrawTriangles(vertices, std::array<uint32_t, 6>{0, 1, 2, 0, 2, 3}, model, image);
    }

private:
    std::unique_ptr<gogl::Shader> shader_;
    std::unique_ptr<gogl::Texture> whiteTexture_;
    std::unique_ptr<gogl::Buffer> vertexBuffer_;
    std::unique_ptr<gogl::Buffer> indicesBuffer_;
    std::unique_ptr<gogl::AttributePointer> attrPtr_;

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