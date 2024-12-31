#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <future>
#include <mutex>

std::mutex coutMutex; // Mutex for synchronized output to std::cout

struct ConfigData {
    std::string key;
    std::string value;
};

// Function to read configuration data from a single file
std::vector<ConfigData> readConfigFile(const std::string& filename) {
    std::vector<ConfigData> configData;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::lock_guard<std::mutex> guard(coutMutex);
        std::cerr << "Error opening file: " << filename << std::endl;
        return configData;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string key, value;

        // Assuming the format is "key=value"
        if (std::getline(ss, key, '=') && std::getline(ss, value)) {
            ConfigData cd;
            cd.key = key;
            cd.value = value;
            configData.push_back(cd);
        }
    }

    return configData;
}

// Function to handle the reading of multiple configuration files
std::vector<ConfigData> readMultipleConfigFiles(const std::vector<std::string>& filenames) {
    std::vector<std::future<std::vector<ConfigData>>> futures;

    for (const auto& filename : filenames) {
        // Launch a thread for each file read operation
        futures.emplace_back(std::async(std::launch::async, readConfigFile, filename));
    }

    std::vector<ConfigData> allConfigData;
    for (auto& future : futures) {
        auto data = future.get(); // Wait for each thread to complete
        allConfigData.insert(allConfigData.end(), data.begin(), data.end());
    }

    return allConfigData;
}

int main() {
    std::vector<std::string> filenames = {
        "config1.txt",
        "config2.txt",
        "config3.txt" // Add your configuration filenames here
    };

    auto allConfigData = readMultipleConfigFiles(filenames);

    // Output collected configuration data
    {
        std::lock_guard<std::mutex> guard(coutMutex);
        for (const auto& config : allConfigData) {
            std::cout << "Key: " << config.key << ", Value: " << config.value << std::endl;
        }
    }

    return 0;
}