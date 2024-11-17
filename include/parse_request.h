#pragma once

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "pokemon_data.h"
#include "util.h"

namespace showdown::request {

std::optional<showdown::data::GameState> GetGameState(const std::vector<std::string>& lines) {
    nlohmann::json request_json;
    for (const std::string& line : lines) {
        std::optional<std::string> command = showdown::util::GetCommand(line);
        if (!command || *command != "request") {
            continue;
        }
        std::cout << "Found request: " << std::endl;
        std::vector<std::string> split_data = showdown::util::SplitLine(line);
        std::cout << "Split data" << std::endl;
        if (split_data.size() < 2) {
            std::cerr << "Split line is less than 2: " << split_data.size() << std::endl;
            continue;
        }
        if (split_data[1].empty()) {
            std::cout << "Does not contain game state. Continuing..." << std::endl;
            continue;
        }
        // Should be index 1
        std::cout << "Parsing data " << split_data[1] << std::endl;
        request_json = nlohmann::json::parse(split_data[1]);
        break;
    }
    if (request_json.empty()) {
        return std::nullopt;
    }
    // Three possible states: switch, active, wait.
    // Currently will only look for active game state.
    showdown::data::GameState state;
    if (!request_json.contains("side")) {
        return std::nullopt;
    }
    if (request_json.contains("wait") && request_json["wait"]) {
        state.wait_for_opponent = true;
    } else {
        state.wait_for_opponent = false;
    }

    if (request_json.contains("active")) {
        for (const auto& move : request_json["active"][0]["moves"]) {
            showdown::data::MoveState move_state;
            move_state.disabled = move["disabled"];
            move_state.pp = move["pp"];
            state.move_options.push_back(move_state);
        }
    }

    for (const auto& mon : request_json["side"]["pokemon"]) {
        showdown::data::PokemonState mon_state;
        {
            mon_state.name = mon["details"];
            mon_state.name = mon_state.name.substr(0, mon_state.name.find(','));
        }
        mon_state.idx = state.pokemon.size();
        if (mon["active"]) {
            state.active_pokemon = mon_state.idx;
        }
        size_t i = 0;
        for (const auto& move : mon["moves"]) {
           mon_state.moves[i] = move;
           i++;
        }
        // TODO: @tj.melanson Populate variables other than FNT and hp
        // TODO: @tj.melanson Handle non FNT status
        std::string condition = mon["condition"];
        if (condition.find("fnt") < condition.size()) {
            std::cout << "Pokemon " << mon_state.name << " is fainted." << std::endl;
            mon_state.status = showdown::data::Status::FNT;
        } else if (condition.find("slp") < condition.size()) {
            mon_state.status = showdown::data::Status::SLP;
        } else {
            mon_state.status = showdown::data::Status::OK;
        }
        mon_state.current_hp = std::stoi(std::string(mon["condition"])); // Should be convertible even with trailing status.

        state.pokemon.push_back(mon_state);
    }
    return state;
}
}
