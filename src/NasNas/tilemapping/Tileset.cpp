/**
* Created by Modar Nasser on 14/07/2020.
**/

#include "NasNas/core/data/Utils.hpp"
#include "NasNas/tilemapping/Tileset.hpp"
#include "NasNas/tilemapping/Tile.hpp"
#ifdef NS_RESLIB
#include "NasNas/reslib/ResourceManager.hpp"
#endif

using namespace ns;
using namespace ns::tm;

TsxTilesetsManager::TsxTilesetsManager() = default;

auto TsxTilesetsManager::get(const std::string& tsx_file_name) -> const TilesetData& {
    static TsxTilesetsManager instance;
    if (instance.m_shared_tilesets.count(tsx_file_name))
        return instance.m_shared_tilesets.at(tsx_file_name);
    else {
        pugi::xml_document xml;
        auto result = xml.load_file(tsx_file_name.c_str());
        if (!result) {
            std::cout << "Error parsing TSX file «" << tsx_file_name << "» : " << result.description() << std::endl;
            std::exit(-1);
        }
        instance.m_shared_tilesets.emplace(tsx_file_name, TilesetData(xml.child("tileset"), utils::path::getPath(tsx_file_name)));
        return instance.m_shared_tilesets.at(tsx_file_name);
    }
}


TilesetData::TilesetData(const pugi::xml_node& xml_node, const std::string& path) :
PropertiesContainer(xml_node.child("properties")),
name(xml_node.attribute("name").as_string()),
tilewidth(xml_node.attribute("tilewidth").as_uint()),
tileheight(xml_node.attribute("tileheight").as_uint()),
tilecount(xml_node.attribute("tilecount").as_uint()),
columns(xml_node.attribute("columns").as_uint()),
margin(xml_node.attribute("margin").as_uint()),
spacing(xml_node.attribute("spacing").as_uint())
{
    m_image_source = xml_node.child("image").attribute("source").as_string();
#ifdef NS_RESLIB
    m_texture = &ns::Res::getTexture(path+m_image_source);
#else
    m_texture = new sf::Texture();
    m_texture->loadFromFile(path + m_image_source);
#endif

    // parsing tileset tiles properties and animations
    for (const auto& xmlnode_tile : xml_node.children("tile")) {
        std::uint32_t tile_id = xmlnode_tile.attribute("id").as_uint();
        m_tiles_data.emplace(tile_id, TileData(xmlnode_tile, this));
    }
}

TilesetData::~TilesetData()
#ifdef NS_RESLIB
    = default;
#else
{
    if(m_texture && m_texture->getSize().x > 0 && m_texture->getSize().y > 0) {
        delete (m_texture);
        m_texture = nullptr;
    }
}
#endif

auto TilesetData::getTexture() const -> const sf::Texture & {
    return *m_texture;
}

auto TilesetData::getTileData(std::uint32_t id) const -> const TileData& {
    if (m_tiles_data.count(id) > 0) return m_tiles_data.at(id);
    m_tiles_data.emplace(id, TileData(id, this));
    return m_tiles_data.at(id);
}

auto TilesetData::getTileTexCoo(std::uint32_t id, Tile::Transformation flip) const -> std::vector<sf::Vector2f> {
    auto texture_rect = getTileTextureRect(id);

    auto coords = std::vector<sf::Vector2f>(4);
    coords[0] = texture_rect.topleft();
    coords[1] = texture_rect.topright();
    coords[2] = texture_rect.bottomright();
    coords[3] = texture_rect.bottomleft();

    if (flip & Tile::VerticalFlip) {
        auto temp = coords[0];
        coords[0].y = coords[3].y;  coords[1].y = coords[2].y;
        coords[3].y = temp.y;       coords[2].y = temp.y;
    }
    if (flip & Tile::HorizontalFlip) {
        auto temp = coords[0];
        coords[0].x = coords[1].x; coords[1].x = temp.x;
        coords[3].x = coords[0].x; coords[2].x = coords[1].x;
    }
    if (flip & Tile::DiagonalFlip) {
        int i1 = 1, i2 = 3;
        if (flip == Tile::Rotation90 ||
            flip == Tile::Rotation270) {
            i1 = 0; i2 = 2;
        }
        auto temp = coords[i1];
        coords[i1] = coords[i2];
        coords[i2] = temp;
    }
    return coords;
}


auto TilesetData::getTileTextureRect(std::uint32_t id) const -> ns::FloatRect {
    auto tx = static_cast<float>((id % columns) * (tilewidth + spacing) + margin);
    auto ty = static_cast<float>((id / columns) * (tileheight + spacing) + margin);
    return {sf::Vector2f(tx, ty), sf::Vector2f(static_cast<float>(tilewidth), static_cast<float>(tileheight))};
}


Tileset::Tileset(const pugi::xml_node& xml_node, const std::string& base_path) :
TilesetData(xml_node, base_path),
firstgid(xml_node.attribute("firstgid").as_uint())
{}

Tileset::Tileset(const TilesetData& tilesetdata, unsigned int first_gid) :
TilesetData(tilesetdata),
firstgid(first_gid)
{}
