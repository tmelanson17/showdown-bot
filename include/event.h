#pragma once

#include <iostream>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "pokemon_data.h"
#include "parse_request.h"

namespace showdown::event {
// TODO: @tj.melanson Merge TurnState into GameState.
enum class TurnState : char {
    kTeamPreview='p',
    kTurn='t',
    kSwitch='s',
    kOpponentSwitch='o',
    kWin='w'
};

std::optional<TurnState> GetTurnState(const std::vector<std::string>& input, const std::string& player_id = "p2") {
    const std::string& last_string = input.at(input.size()-1);
    std::optional<std::string> maybe_command = showdown::util::GetCommand(last_string);
    if (!maybe_command.has_value()) {
        return std::nullopt;
    }
    std::string command = maybe_command.value();
    if (command == "teampreview") {
        return TurnState::kTeamPreview;
    } else if (command == "turn") {
        return TurnState::kTurn;
    } else if (command == "faint") {
        std::optional<std::string> previous_command = showdown::util::GetCommand(input.at(input.size()-2));
        // Case 1: Both mons fainted. Assumes 2 players, 2 mons on the field.
        if (previous_command.has_value() && *previous_command == "faint") {
            return TurnState::kSwitch;
        }
        // Case 2: Player fainted, Opponent didn't. 
        else if (last_string.find(player_id) != last_string.size()) {
            return TurnState::kSwitch;
        }
        // Case 3: Opponent fainted, Player didn't.
        else {
            return TurnState::kOpponentSwitch;
        }
    } else if (command == "win") {
        return TurnState::kWin;
    } else {
        return std::nullopt;
    }
}

// TODO: @tj.melanson Move agent / action into their own file.
struct Action {
    enum Type {
        Move=0,
        Switch=1,
        Team=2,
    };

    std::string ToString() {
        if (action_type == Type::Move) {
            return "move " + std::to_string(action_idx);
        } else if (action_type == Type::Switch) {
            return "switch " + std::to_string(action_idx);
        } else if (action_type == Type::Team) {
            // Assume the integers are smushed together.
            return "team " + std::to_string(action_idx);
        }
        return "";
    }

    Type action_type;
    int action_idx;
};


class RandomAgent {
public:
    RandomAgent() : rng(dev()) {}

    Action Choose(const showdown::data::GameState& state, const TurnState& turn_state) {
        if (turn_state == TurnState::kTeamPreview) {
            std::cout << "Creating team..." << std::endl;
            std::uniform_int_distribution<std::mt19937::result_type> team_dist(1, state.pokemon.size());
            std::set<int> chosen_pokemon;
            int idx=0;
            // TODO: @tj.melanson Change for variable team preview size.
            while (chosen_pokemon.size() < 3) {
                int new_idx = team_dist(rng);

                if (chosen_pokemon.find(new_idx) == chosen_pokemon.end()) {
                    chosen_pokemon.insert(new_idx);
                    idx = idx*10 + new_idx;
                }
            }
            return Action {
                .action_type=Action::Team,
                .action_idx=idx,
            };
        }
        std::vector<int> switchins;
        for (const showdown::data::PokemonState& mon : state.pokemon) {
            std::cout << "Mon: " << mon.name << " Status: " << char(mon.status) << std::endl;
            if (mon.status != showdown::data::Status::FNT && mon.idx != state.active_pokemon) {
                switchins.push_back(mon.idx+1);
            }
        }
        std::uniform_real_distribution<> action_dist(0., 1.);
        // Prefer move 80% of the time, or if there are no switch-ins
        if (turn_state == TurnState::kTurn && (switchins.size() == 0 || action_dist(rng) < 0.8)) {
            std::cout << "Choosing fight option..." << std::endl;
            std::vector<int> available_moves;

            // Check which moves are available.
            for (int i = 0; i < state.move_options.size(); ++i) {
                const auto& move = state.move_options[i];
                if (!move.disabled && move.pp > 0) {
                    available_moves.push_back(i+1);
                }
            }
            if (available_moves.size() > 0) {
                std::uniform_int_distribution<std::mt19937::result_type> move_dist(0,available_moves.size()-1);
                size_t index = static_cast<size_t>(move_dist(rng));
                int random_move = available_moves[index];
                std::cout <<  index << " " << random_move << std::endl;
                return Action {
                    .action_type = Action::Move,
                    .action_idx = random_move,
                };
            } else {
                std::cout << "No available moves" << std::endl;
            }
        }
        std::cout << "Switching..." << std::endl;
        std::uniform_int_distribution<std::mt19937::result_type> switch_dist(0, switchins.size()-1);
        return Action {
            .action_type = Action::Switch,
            .action_idx = switchins[static_cast<size_t>(switch_dist(rng))],
        };
    }
private:
    std::random_device dev;
    std::mt19937 rng;
};
}
