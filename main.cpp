#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <unordered_map>
#include <set>
#include <functional>

using namespace std;

const char* DB_FILE = "database.dat";
const int MAX_KEY_LEN = 65;

struct Record {
    char key[MAX_KEY_LEN];
    int value;
    bool deleted;
};

// Custom hash for pair<size_t, int>
struct PairHash {
    size_t operator()(const pair<size_t, int>& p) const {
        return p.first ^ (p.second << 1);
    }
};

class FileDatabase {
private:
    fstream file;
    unordered_map<pair<size_t, int>, streampos, PairHash> index; // (key_hash, value) -> file_position
    hash<string> hasher;
    
    void openFile() {
        file.open(DB_FILE, ios::in | ios::out | ios::binary);
        if (!file.is_open()) {
            // Create file if it doesn't exist
            ofstream newFile(DB_FILE, ios::binary);
            newFile.close();
            file.open(DB_FILE, ios::in | ios::out | ios::binary);
        }
    }
    
    void buildIndex() {
        // Build index by scanning the file once at startup
        file.clear();
        file.seekg(0, ios::beg);
        
        Record rec;
        while (file.read((char*)&rec, sizeof(Record))) {
            if (!rec.deleted) {
                streampos pos = file.tellg();
                pos -= sizeof(Record);
                size_t key_hash = hasher(string(rec.key));
                index[make_pair(key_hash, rec.value)] = pos;
            }
        }
    }
    
public:
    FileDatabase() {
        openFile();
        buildIndex();
    }
    
    ~FileDatabase() {
        if (file.is_open()) {
            file.close();
        }
    }
    
    void insert(const string& index_key, int value) {
        // Check if record already exists in index
        size_t key_hash = hasher(index_key);
        auto key = make_pair(key_hash, value);
        if (index.find(key) != index.end()) {
            return; // Already exists
        }
        
        // Add new record at the end of file
        file.clear();
        file.seekp(0, ios::end);
        streampos pos = file.tellp();
        
        Record newRec;
        strncpy(newRec.key, index_key.c_str(), MAX_KEY_LEN - 1);
        newRec.key[MAX_KEY_LEN - 1] = '\0';
        newRec.value = value;
        newRec.deleted = false;
        
        file.write((char*)&newRec, sizeof(Record));
        file.flush();
        
        // Update index
        index[key] = pos;
    }
    
    void remove(const string& index_key, int value) {
        size_t key_hash = hasher(index_key);
        auto key = make_pair(key_hash, value);
        auto it = index.find(key);
        if (it == index.end()) return;
        
        streampos pos_to_delete = it->second;
        
        // Mark record as deleted in file
        file.clear();
        file.seekg(pos_to_delete);
        Record rec;
        file.read((char*)&rec, sizeof(Record));
        rec.deleted = true;
        
        file.seekp(pos_to_delete);
        file.write((char*)&rec, sizeof(Record));
        file.flush();
        
        // Update index
        index.erase(it);
    }
    
    void find(const string& index_key) {
        size_t key_hash = hasher(index_key);
        vector<int> values;
        
        for (auto& kv : index) {
            if (kv.first.first == key_hash) {
                values.push_back(kv.first.second);
            }
        }
        
        if (values.empty()) {
            cout << "null" << endl;
        } else {
            sort(values.begin(), values.end());
            for (size_t i = 0; i < values.size(); i++) {
                if (i > 0) cout << " ";
                cout << values[i];
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
