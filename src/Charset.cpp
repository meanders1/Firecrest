#include "Charset.h"

#include "msdf-atlas-gen/msdf-atlas-gen.h"
#include <stdexcept>

fc::Charset::Charset(const std::string& fontFile)
{

    const double pixelSize = 32.0;

    msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
    if (!ft)
        throw std::runtime_error("Failed to initialize FreeType");

    msdfgen::FontHandle* font = msdfgen::loadFont(ft, fontFile.c_str());
    if (!font)
        throw std::runtime_error("Failed to load font");

    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    msdf_atlas::FontGeometry fontGeometry(&glyphs);

    fontGeometry.loadCharset(font, 1.0f, msdf_atlas::Charset::ASCII);

    for (msdf_atlas::GlyphGeometry& glyph : glyphs)
        glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, 3.0, 0);

    msdf_atlas::TightAtlasPacker packer;

    packer.setDimensionsConstraint(msdf_atlas::DimensionsConstraint::SQUARE);
    packer.setMinimumScale(pixelSize);

    // If updating the pixel range, remember to update the corresponding value
    // in the TextRenderer fragment shader!
    packer.setPixelRange(2.0);
    packer.setMiterLimit(1.0);

    packer.pack(glyphs.data(), glyphs.size());

    int width = 0;
    int height = 0;

    packer.getDimensions(width, height);

    msdf_atlas::ImmediateAtlasGenerator<float, 3, msdf_atlas::msdfGenerator,
                                        msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>>
        generator(width, height);

    msdf_atlas::GeneratorAttributes attributes;
    generator.setAttributes(attributes);
    generator.setThreadCount(4);

    generator.generate(glyphs.data(), glyphs.size());

    auto& storage = generator.atlasStorage();
    msdfgen::BitmapConstRef<msdfgen::byte, 3> bitmap = storage;

    // Use these for normalization, not the packer variables
    float actualWidth = (float)bitmap.width;
    float actualHeight = (float)bitmap.height;

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    _atlas = fc::gl::Texture2D(GL_RGB8, actualWidth, actualHeight, GL_RGB, GL_UNSIGNED_BYTE,
                               bitmap.pixels);

    for (const msdf_atlas::GlyphGeometry& g : glyphs) {
        char c = static_cast<char>(g.getCodepoint());
        if (c < 0 || c > 127)
            continue;

        double l, b, r, t;
        g.getQuadPlaneBounds(l, b, r, t);

        double u0, v0, u1, v1;
        g.getQuadAtlasBounds(u0, v0, u1, v1);

        Glyph glyph;

        // Convert from pixel-space to normalized OpenGL UV
        glyph.uvMin = glm::vec2(u0 / actualWidth, v0 / actualHeight);
        glyph.uvMax = glm::vec2(u1 / actualWidth, v1 / actualHeight);
        glyph.size = {float(r - l), float(t - b)};
        glyph.bearing = {float(l), float(t)};
        glyph.advance = float(g.getAdvance());

        _glyphs[(unsigned char)c] = glyph;
    }

    const msdfgen::FontMetrics& metrics = fontGeometry.getMetrics();

    _ascender = metrics.ascenderY;
    _descender = metrics.descenderY;
    _lineHeight = metrics.lineHeight;

    msdfgen::destroyFont(font);
    msdfgen::deinitializeFreetype(ft);
}

const fc::Charset::Glyph& fc::Charset::glyph(char c) const
{
    return _glyphs.at((unsigned char)c);
}

const fc::gl::Texture2D& fc::Charset::atlas() const
{
    return _atlas;
}

float fc::Charset::ascender() const
{
    return _ascender;
}

float fc::Charset::descender() const
{
    return _descender;
}

float fc::Charset::lineHeight() const
{
    return _lineHeight;
}