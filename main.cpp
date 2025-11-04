#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <map>
#include <set>

using namespace std;

const char* DB_FILE = "database.dat";
const int MAX_KEY_LEN = 65;

// In-memory structure for managing index-value pairs
class FileDatabase {
private:
    map<string, set<int>> data;
    bool loaded;
    
    void load() {
        ifstream file(DB_FILE, ios::binary);
        if (!file.is_open()) {
            loaded = true;
            return;
        }
        
        int count;
        file.read((char*)&count, sizeof(count));
        
        for (int i = 0; i < count; i++) {
            int keyLen;
            file.read((char*)&keyLen, sizeof(keyLen));
            
            char key[MAX_KEY_LEN];
            file.read(key, keyLen);
            key[keyLen] = '\0';
            
            int valueCount;
            file.read((char*)&valueCount, sizeof(valueCount));
            
            for (int j = 0; j < valueCount; j++) {
                int value;
                file.read((char*)&value, sizeof(value));
                data[string(key)].insert(value);
            }
        }
        
        file.close();
        loaded = true;
    }
    
    void save() {
        ofstream file(DB_FILE, ios::binary | ios::trunc);
        
        int count = data.size();
        file.write((char*)&count, sizeof(count));
        
        for (auto& pair : data) {
            int keyLen = pair.first.length();
            file.write((char*)&keyLen, sizeof(keyLen));
            file.write(pair.first.c_str(), keyLen);
            
            int valueCount = pair.second.size();
            file.write((char*)&valueCount, sizeof(valueCount));
            
            for (int value : pair.second) {
                file.write((char*)&value, sizeof(value));
            }
        }
        
        file.close();
    }
    
public:
    FileDatabase() : loaded(false) {
        load();
    }
    
    ~FileDatabase() {
        save();
    }
    
    void insert(const string& index, int value) {
        if (!loaded) load();
        data[index].insert(value);
    }
    
    void remove(const string& index, int value) {
        if (!loaded) load();
        auto it = data.find(index);
        if (it != data.end()) {
            it->second.erase(value);
            if (it->second.empty()) {
                data.erase(it);
            }
        }
    }
    
    void find(const string& index) {
        if (!loaded) load();
        auto it = data.find(index);
        if (it == data.end() || it->second.empty()) {
            cout << "null" << endl;
        } else {
            bool first = true;
            for (int value : it->second) {
                if (!first) cout << " ";
                cout << value;
                first = false;
            }
            cout << endl;
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    
    int n;
    cin >> n;
    
    FileDatabase db;
    
    for (int i = 0; i < n; i++) {
        string cmd;
        cin >> cmd;
        
        if (cmd == "insert") {
            string index;
            int value;
            cin >> index >> value;
            db.insert(index, value);
        } else if (cmd == "delete") {
            string index;
            int value;
            cin >> index >> value;
            db.remove(index, value);
        } else if (cmd == "find") {
            string index;
            cin >> index;
            db.find(index);
        }
    }
    
    return 0;
}
