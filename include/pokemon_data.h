#pragma once 

#include <array>
#include <string>

namespace showdown::data {

// For now, the pokemon name and moves will be strings.
// TODO: figure out where to add encoding for moves / pokemon
struct Pokemon {
    std::string name = "Unknown";
    std::array<std::string, 4> moves;
    // Flag for min attack; otherwise all 6 stats are maxed out in Gen 1
    bool min_attack = false; 
    // Level not implemented yet, assume all are at 25
    int level = 25;
    // Ability, Nature not implemented in Gen 1.
};

// Representation of the full team to upload for battles
struct Team {
    std::array<Pokemon, 6> mons;
};

enum class Status : char {
    OK='o',
    PZ='p',
    BRN='b',
    SLP='s',
    PSN='n',
    FRZ='z',
    FNT='f',
};

// State of the move.
// TODO: @tj.melanson Add data about the move itself.
struct MoveState {
    size_t pp=0;
    bool disabled=true;
};

struct PokemonState {
    size_t idx;
    std::string name;
    int current_hp;
    int total_hp;
    std::array<std::string, 4> moves;
    Status status = Status::OK;
};

struct GameState { 
    std::vector<PokemonState> pokemon;
    std::vector<MoveState> move_options;
    size_t active_pokemon;
    bool wait_for_opponent;
};

}  // namespace showdown::data
