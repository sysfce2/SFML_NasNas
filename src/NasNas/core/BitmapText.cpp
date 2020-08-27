/**
* Created by Modar Nasser on 25/06/2020.
**/


#include "NasNas/core/BitmapText.hpp"

using namespace ns;

BitmapGlyph::BitmapGlyph(const ns::IntRect& texture_rect, char character, int spacing) :
texture_rect(texture_rect),
character(character),
advance(spacing)
{}


BitmapFont::BitmapFont(const sf::Texture& texture, const sf::Vector2u& glyph_size) :
BitmapFont(texture, glyph_size, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", {})
{}

BitmapFont::BitmapFont(const sf::Texture& texture, const sf::Vector2u& glyph_size, const std::string& chars_map) :
BitmapFont(texture, glyph_size, chars_map, {})
{}

BitmapFont::BitmapFont(const sf::Texture& texture, const sf::Vector2u& glyph_size, const std::unordered_map<char, int>& spacings_map) :
BitmapFont(texture, glyph_size, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", spacings_map)
{}

BitmapFont::BitmapFont(const sf::Texture& texture, const sf::Vector2u& glyph_size, const std::string& chars_map, const std::unordered_map<char, int>& spacings_map) :
m_texture(&texture),
m_glyph_size(glyph_size) {
    m_chars_map = chars_map;
    m_advance_map = spacings_map;

    unsigned int i = 0;
    for (int y = 0; y < (int)m_texture->getSize().y; y += m_glyph_size.y) {
        for (int x = 0; x < (int)m_texture->getSize().x; x += m_glyph_size.x) {
            if (i < m_chars_map.size()) {
                char character = m_chars_map[i];
                int spacing = m_glyph_size.x;
                if (m_advance_map.count(character) > 0)
                    spacing = m_advance_map[character];
                m_glyphs[character] = new BitmapGlyph({{x, y}, {(int)m_glyph_size.x, (int)m_glyph_size.y}}, character, spacing);
            }
            i += 1;
        }
    }
}

BitmapFont::~BitmapFont() {
    for (const auto& [name, glyph] : m_glyphs) {
        delete(glyph);
    }
}

auto BitmapFont::getGlyphSize() -> const sf::Vector2u& {
    return m_glyph_size;
}

auto BitmapFont::getTexture() -> const sf::Texture* {
    return m_texture;
}

auto BitmapFont::getGlyph(char character) -> const BitmapGlyph& {
    if (m_glyphs.count(character) > 0) {
        return *m_glyphs[character];
    }
    std::cout << "Selected BitmapFont does not have glyph for character «" << character << "» " << std::endl;
    exit(-1);
}

auto BitmapFont::computeStringSize(const std::string& string) -> sf::Vector2i {
    sf::Vector2i result = {0, (int)m_glyph_size.y};
    for (const auto character : string) {
        result.x += getGlyph(character).advance;
    }
    return result;
}


BitmapText::BitmapText(const std::string& text) {
    m_string = text;
    m_vertices.setPrimitiveType(sf::PrimitiveType::Quads);
}

void BitmapText::setString(const std::string &string) {
    m_string = string;
    refresh();
}

auto BitmapText::getFont() -> std::shared_ptr<BitmapFont>& {
    return m_font;
}

void BitmapText::setFont(const std::shared_ptr<BitmapFont>& font) {
    m_font = font;
    refresh();
}

void BitmapText::setColor(const sf::Color &color) {
    m_color = color;
    refresh();
}

auto BitmapText::getPosition() -> sf::Vector2f {
    return m_transformable.getPosition();
}

void BitmapText::setPosition(const sf::Vector2f& position) {
    m_transformable.setPosition(position);
}

void BitmapText::setPosition(float x, float y) {
    m_transformable.setPosition(x, y);
}

auto BitmapText::getGlobalBounds() -> ns::FloatRect {
    return ns::FloatRect(m_transformable.getPosition(), getSize());
}

auto BitmapText::getWidth() const -> int {
    return m_width;
}

auto BitmapText::getHeight() const -> int {
    return m_height;
}

auto BitmapText::getSize() const -> sf::Vector2f {
    return sf::Vector2f((float)getWidth(), (float)getHeight());
}

void BitmapText::move(float offsetx, float offsety) {
    m_transformable.move(offsetx, offsety);
}

void BitmapText::refresh() {
    m_vertices.clear();
    if (m_font != nullptr) {
        float x = 0, y = 0;
        int w = 0, h = m_font->getGlyphSize().y;
        int max_w = 0;
        for(const auto& character : m_string) {
            if (character == '\n') {
                x = 0;
                y += (float)m_font->getGlyphSize().y;
                max_w = std::max(max_w, w);
                w = 0;
                h += m_font->getGlyphSize().y;
                continue;
            }
            auto glyph = m_font->getGlyph(character);
            auto glyph_size = (sf::Vector2f)m_font->getGlyphSize();
            auto vertex_tl = sf::Vertex({x, y}, m_color, (sf::Vector2f)glyph.texture_rect.topleft());
            auto vertex_tr = sf::Vertex({x + glyph_size.x, y}, m_color, (sf::Vector2f)glyph.texture_rect.topright());
            auto vertex_br = sf::Vertex({x + glyph_size.x, y + glyph_size.y}, m_color, (sf::Vector2f)glyph.texture_rect.bottomright());
            auto vertex_bl = sf::Vertex({x, y + glyph_size.y}, m_color, (sf::Vector2f)glyph.texture_rect.bottomleft());
            m_vertices.append(vertex_tl);
            m_vertices.append(vertex_tr);
            m_vertices.append(vertex_br);
            m_vertices.append(vertex_bl);
            x += (float)glyph.advance;
            w += glyph.advance;
        }
        m_width = std::max(max_w, w);
        m_height = h;
    }
}

void BitmapText::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    if (m_font != nullptr) {
        states.texture = m_font->getTexture();
        states.transform *= m_transformable.getTransform();
        target.draw(m_vertices, states);
    }
}
