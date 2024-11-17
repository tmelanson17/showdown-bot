#pragma once 

#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>

#include "pokemon_data.h"

namespace showdown::config {

enum class ConfigStage {
    kPokemonName=0,
    kAbility=1,
    kLevel=2,
    kEvs,
    kNature,
    kMove1,
    kMove2,
    kMove3,
    kMove4,
    kEnd,
};

constexpr size_t FindFirstAlphaNumeric(const std::string& s) {
    return std::find_if(s.begin(), s.end(), [](char c){return std::isalpha(c);}) - s.begin();
}

std::string RemoveWhitespace(const std::string& input) {
    size_t i = 0;
    std::string tmp = input;
    while (i < tmp.size()) {
        if (tmp[i] == ' ') {
            tmp.erase(i, 1);
        } else {
            i++;
        }
    }
    return tmp;
}

std::optional<showdown::data::Team> ParsePokepaste(const std::filesystem::path& pokepaste) {
    if (!std::filesystem::exists(pokepaste)) {
        std::cerr << "File not found: " << pokepaste << std::endl;
        return std::nullopt;
    }
    showdown::data::Team team;
    std::ifstream reader(pokepaste, std::ifstream::in);
    ConfigStage current_stage = ConfigStage::kPokemonName;
    size_t pokemon_idx = 0;
    std::string line;
    while (std::getline(reader, line)) {
        switch (current_stage) {
            case ConfigStage::kPokemonName: {
                team.mons[pokemon_idx].name = RemoveWhitespace(line);
                break;
            } 
            case ConfigStage::kLevel: {
                // Trim last 2 whitespaces.
                line = line.substr(0, line.size() - 2);
                team.mons[pokemon_idx].level = std::stoi(line.substr(line.find(" ") + 1));
                break;
            }
            case ConfigStage::kEvs: {
                team.mons[pokemon_idx].min_attack =  (line.find("Atk") == line.size());
                break;
            }
            case ConfigStage::kMove1:
            case ConfigStage::kMove2:
            case ConfigStage::kMove3:
            case ConfigStage::kMove4: {
                size_t i_move = size_t(current_stage) - size_t(ConfigStage::kMove1);
                team.mons[pokemon_idx].moves[i_move] = line.substr(FindFirstAlphaNumeric(line));
                break;
            }
            case ConfigStage::kEnd: {
                pokemon_idx++;
                break;
            }
            case ConfigStage::kAbility:
            case ConfigStage::kNature:
                break;
        }
        if (current_stage == ConfigStage::kEnd) {
            current_stage = ConfigStage::kPokemonName;
        } else {
            current_stage = static_cast<ConfigStage>(size_t(current_stage) + 1);
        }
    }
    return team;
}

// Currently only supports Gen 1. Will need to add ability, etc.
bool ToPokepaste(const showdown::data::Team& team, const std::filesystem::path& pokepaste) {
    std::ofstream writer(pokepaste);
    ConfigStage current_stage = ConfigStage::kPokemonName;
    for (const showdown::data::Pokemon& pokemon: team.mons) {
        writer << pokemon.name << std::endl;
        writer << "Ability: No Ability" << std::endl;
        writer << "Level: " << std::to_string(pokemon.level) << "  " << std::endl;
        writer << "EVs: 252 HP /";
        if (!pokemon.min_attack) {
            writer << " 252 Atk /";
        }
        writer << " 252 Def / 252 SpA / 252 Spe" << std::endl;
        writer << "Serious Nature" << std::endl;
        for (const std::string& move : pokemon.moves) {
            if (!move.empty()) {
                writer << "- " << move << std::endl;
            }
        }
        writer << std::endl;
    }
    return true;
}

std::string ToPokepastePacked(const showdown::data::Team& team)  {
    std::stringstream writer;
    bool first_mon = true;
    for (const showdown::data::Pokemon& pokemon : team.mons) {
        if (!first_mon) {
            writer << "]";
        } else {
            first_mon = false;
        }
        writer << pokemon.name << "|||";
        writer << "NoAbility|";
        bool first_move = true;
        for (const std::string& move : pokemon.moves) {
            if (!first_move) {
                writer << ",";
            } else {
                first_move = false;
            }
            if (!move.empty()) {
                writer << RemoveWhitespace(move); 
            }
        }
        writer << "|";
        writer << "Serious";
        writer << "|";
        bool first_ev = true;
        std::string atk_evs = (pokemon.min_attack) ? "" : "252";
        for (const std::string& ev : std::vector<std::string>({"252", atk_evs, "252", "252", "", "252"})) {
            if (!first_ev) {
                writer << ",";
            } else {
                first_ev = false;
            }
            writer << ev;
        }
        writer << "||||";
        writer << std::to_string(pokemon.level);
        writer << "|";
    }
    return writer.str();
}

}  // namespace showdown::config
