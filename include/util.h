#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace showdown::util {

std::optional<std::string> GetCommand(const std::string& line) {
    std::cout << line << std::endl;
    size_t first_bar = line.find('|')+1;
    if (first_bar >= line.size()) {
        return std::nullopt;
    } 
    size_t second_bar = line.substr(first_bar).find('|');
    std::cout << first_bar << " " << second_bar << std::endl;
    if (second_bar >= line.size()) {
        return std::nullopt;
    }
    std::cout << line.substr(first_bar, second_bar) << std::endl;
    return line.substr(first_bar, second_bar);
}

std::vector<std::string> SplitLine(const std::string& line) {
    std::vector<std::string> result;
    size_t bar_idx = line.find('|');
    while (bar_idx < line.size()) {
        std::string substring = line.substr(bar_idx+1);
        size_t second_bar = std::min(substring.find('|'), substring.find('>'));
        std::cout << "Substring from " << bar_idx+1 << " to " << bar_idx+1+second_bar << std::endl;
        result.push_back(line.substr(bar_idx+1, second_bar));
        bar_idx = std::max(second_bar, bar_idx+1+second_bar);
    }
    return result;
}

}