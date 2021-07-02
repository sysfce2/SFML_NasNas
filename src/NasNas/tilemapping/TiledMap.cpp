/**
* Created by Modar Nasser on 09/07/2020.
**/

#include "NasNas/core/data/Config.hpp"
#include "NasNas/core/data/Utils.hpp"
#include "NasNas/tilemapping/TiledMap.hpp"
#include "NasNas/tilemapping/TileLayer.hpp"

using namespace ns;
using namespace ns::tm;

TiledMap::TiledMap() = default;

auto TiledMap::loadFromFile(const std::string& file_name) -> bool {
    pugi::xml_document xml;
    auto result = xml.load_file(file_name.c_str());
    if (!result) {
        std::cout << "Error parsing TMX file «" << file_name << "» : " << result.description() << std::endl;
        return false;
    }
    m_file_name = ns::utils::path::getFilename(file_name);
    m_file_relative_path = ns::utils::path::getPath(file_name);
    load(xml);
    return true;
}

auto TiledMap::loadFromString(const std::string& data) -> bool {
    pugi::xml_document xml;
    auto result = xml.load_string(data.c_str());
    if (!result) {
        std::cout << "Error parsing TMX data : " << result.description() << std::endl;
        return false;
    }
    m_file_name = "";
    m_file_relative_path = "";
    load(xml);
    return true;
}

void TiledMap::load(const pugi::xml_document& xml) {
    m_xmlnode_map = xml.child("map");
    m_size.x = m_xmlnode_map.attribute("width").as_uint();
    m_size.y = m_xmlnode_map.attribute("height").as_uint();
    m_tilesize.x = m_xmlnode_map.attribute("tilewidth").as_uint();
    m_tilesize.y = m_xmlnode_map.attribute("tileheight").as_uint();

    parseProperties(m_xmlnode_map.child("properties"));

    // parse tilesets
    m_tilesets.reserve(std::distance(m_xmlnode_map.children("tileset").begin(), m_xmlnode_map.children("tileset").end()));
    m_tilesets_data.reserve(5);
    for (const auto& xmlnode_tileset : m_xmlnode_map.children("tileset")) {
        unsigned int firstgid = xmlnode_tileset.attribute("firstgid").as_uint();
        // external tileset
        if (xmlnode_tileset.attribute("source")){
            const auto& tsx_tileset = TsxTilesetsManager::get(m_file_relative_path + xmlnode_tileset.attribute("source").as_string());
            m_tilesets.emplace_back(tsx_tileset, firstgid);
        }
        // embedded tileset
        else {
            m_tilesets_data.emplace_back(xmlnode_tileset, m_file_relative_path);
            m_tilesets.emplace_back(m_tilesets_data.back(), firstgid);
        }
    }

    // parse tile layers
    for (const auto& xmlnode_layer : m_xmlnode_map.children("layer")) {
        addTileLayer(xmlnode_layer, this);
    }
    // parse object layers
    for (const auto& xmlnode_layer : m_xmlnode_map.children("objectgroup")) {
        addObjectLayer(xmlnode_layer, this);
    }
    for (const auto& xmlnode_layer : m_xmlnode_map.children("group")) {
        addObjectLayer(xmlnode_layer, this);
    }
}

auto TiledMap::getSize() const -> sf::Vector2f {
    return sf::Vector2f(m_size.x * m_tilesize.x, m_size.y * m_tilesize.y);
}

auto TiledMap::getGridSize() const -> const sf::Vector2u& {
    return m_size;
}

auto TiledMap::getTileSize() const -> const sf::Vector2u& {
    return m_tilesize;
}

auto TiledMap::allTilesets() const -> const std::vector<Tileset>& {
    return m_tilesets;
}

auto TiledMap::getTileTileset(unsigned int gid) const -> const Tileset& {
    for (const auto& tileset : m_tilesets) {
        if (tileset.firstgid <= gid && gid < tileset.firstgid + tileset.data.tilecount)
            return tileset;
    }
    std::cout << "Error (TiledMap::getTileTileset) : Tile gid " << gid << " not found in any tileset" << std::endl;
    exit(-1);
}

void TiledMap::update() {
    for (auto& [name, layer] : allTileLayers())
        layer->update();
}
