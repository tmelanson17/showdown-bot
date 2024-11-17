#include <iostream>
#include <fstream>
#include <unistd.h>
#include <cstring>
#include <cerrno>
#include <sys/types.h>
#include <sys/stat.h>

#include "event.h"
#include "parse_text.h"
#include "pokemon_data.h"
#include "process_io.h"

inline std::vector<const char*> GetShowdownCommand() {
    return {
        "/Users/tjmelanson/development/downloaded-code/pokemon-showdown/pokemon-showdown",
        "simulate-battle"
    };
}

int main() {
    // Get Team
    std::filesystem::path full_path("/Users/tjmelanson/development/pokemans/showdown-bot/config/sample_team.txt");
    auto team = showdown::config::ParsePokepaste(full_path);
    if (!team.has_value()) {
        std::cerr << "Couldn't parse team" << std::endl;
        return 1;
    }

    // Create FIFOs (named pipes)
    showdown::io::FIFO fifo;
    if (fifo.GetErrorCode() != 0) {
        return 1;
    }

    showdown::event::RandomAgent agent;
    showdown::data::GameState state;

    std::cout << "Opening FIFO" << std::endl;

    {
        std::ofstream fifo_out = fifo.CreateOutputStream();
        if (!fifo_out) {
            return 1;
        }
        std::cout << "Sending pokepaste" << std::endl;
        
        // Initial round: Just upload team
        fifo_out << "{\"team\": \"" << showdown::config::ToPokepastePacked(*team) << "\"}" << std::endl;
        std::cout << "{\"team\": \"" << showdown::config::ToPokepastePacked(*team) << "\"}" << std::endl;
        fifo_out.close();
    }

    std::cout << "Entering main loop" << std::endl;
    while (true) {
        // Read the output from FIFO (output from the child process)
        std::vector<std::string> lines;
        std::string line;
        {
            std::ifstream fifo_in = fifo.CreateInputStream();
            if (!fifo_in) {
                return 1;
            }
            while (std::getline(fifo_in, line)) {
                std::cout << "Output from child process: " << line << std::endl;
                lines.push_back(line);
            }
            fifo_in.close();
        }
        if (lines.size() == 0) {
            std::cout << "No output" << std::endl;
            continue;
        }

        std::optional<showdown::event::TurnState> turn = showdown::event::GetTurnState(lines);
        if (!turn.has_value()) {
            std::cout << "No discernible state" << std::endl;
            continue;
        }
        std::cout << "last turn: " << int(turn.value()) << std::endl;
        std::optional<showdown::data::GameState> optional_state = showdown::request::GetGameState(lines);
        if (optional_state.has_value()) {
            state = *optional_state;
        } else {
            std::cout << "State not parseable" << std::endl;
            continue;
        }
        if (state.wait_for_opponent) {
            std::cout << "Waiting for opponent switchin" << std::endl;
            continue;
        }
        {
            std::ofstream fifo_out = fifo.CreateOutputStream();
            if (!fifo_out) {
                return 1;
            }
            fifo_out << agent.Choose(state, *turn).ToString() << std::endl;
            fifo_out.close();
        }
    }
}

