#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>
#include <filesystem>

namespace fs = std::filesystem;

class FileStorageBPT {
private:
    static const std::string DATA_FILE;
    static const int MAX_KEY_LENGTH = 64;
    
    // In-memory cache for performance
    std::map<std::string, std::set<int>> data;
    bool modified = false;
    
public:
    FileStorageBPT() {
        load_from_file();
    }
    
    ~FileStorageBPT() {
        if (modified) {
            save_to_file();
        }
    }
    
    void insert(const std::string& key, int value) {
        auto& values = data[key];
        if (values.find(value) == values.end()) {
            values.insert(value);
            modified = true;
        }
    }
    
    void erase(const std::string& key, int value) {
        auto it = data.find(key);
        if (it != data.end()) {
            if (it->second.erase(value) > 0) {
                modified = true;
                if (it->second.empty()) {
                    data.erase(it);
                }
            }
        }
    }
    
    std::vector<int> find(const std::string& key) const {
        auto it = data.find(key);
        if (it != data.end()) {
            return std::vector<int>(it->second.begin(), it->second.end());
        }
        return {};
    }
    
private:
    void load_from_file() {
        if (!fs::exists(DATA_FILE)) {
            return;
        }
        
        std::ifstream file(DATA_FILE, std::ios::binary);
        if (!file.is_open()) {
            return;
        }
        
        // Read number of entries
        size_t entry_count;
        file.read(reinterpret_cast<char*>(&entry_count), sizeof(entry_count));
        
        for (size_t i = 0; i < entry_count; ++i) {
            // Read key length
            size_t key_length;
            file.read(reinterpret_cast<char*>(&key_length), sizeof(key_length));
            
            // Read key
            std::string key(key_length, '\0');
            file.read(&key[0], key_length);
            
            // Read number of values for this key
            size_t value_count;
            file.read(reinterpret_cast<char*>(&value_count), sizeof(value_count));
            
            // Read values
            for (size_t j = 0; j < value_count; ++j) {
                int value;
                file.read(reinterpret_cast<char*>(&value), sizeof(value));
                data[key].insert(value);
            }
        }
        
        file.close();
        modified = false;
    }
    
    void save_to_file() {
        std::ofstream file(DATA_FILE, std::ios::binary);
        if (!file.is_open()) {
            return;
        }
        
        // Calculate total number of entries
        size_t total_entries = 0;
        for (const auto& pair : data) {
            total_entries += pair.second.size();
        }
        
        // Write total number of entries
        file.write(reinterpret_cast<const char*>(&total_entries), sizeof(total_entries));
        
        // Write each key-value pair
        for (const auto& pair : data) {
            const std::string& key = pair.first;
            const std::set<int>& values = pair.second;
            
            for (int value : values) {
                // Write key length
                size_t key_length = key.length();
                file.write(reinterpret_cast<const char*>(&key_length), sizeof(key_length));
                
                // Write key
                file.write(key.c_str(), key_length);
                
                // Write value (as single value entry)
                size_t value_count = 1;
                file.write(reinterpret_cast<const char*>(&value_count), sizeof(value_count));
                file.write(reinterpret_cast<const char*>(&value), sizeof(value));
            }
        }
        
        file.close();
        modified = false;
    }
};

const std::string FileStorageBPT::DATA_FILE = "bpt_data.bin";

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    FileStorageBPT bpt;
    
    int n;
    std::cin >> n;
    
    for (int i = 0; i < n; ++i) {
        std::string command;
        std::cin >> command;
        
        if (command == "insert") {
            std::string key;
            int value;
            std::cin >> key >> value;
            bpt.insert(key, value);
        } else if (command == "delete") {
            std::string key;
            int value;
            std::cin >> key >> value;
            bpt.erase(key, value);
        } else if (command == "find") {
            std::string key;
            std::cin >> key;
            
            std::vector<int> values = bpt.find(key);
            if (values.empty()) {
                std::cout << "null";
            } else {
                for (size_t j = 0; j < values.size(); ++j) {
                    if (j > 0) std::cout << " ";
                    std::cout << values[j];
                }
            }
            std::cout << "\n";
        }
    }
    
    return 0;
}