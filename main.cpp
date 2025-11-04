#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <map>
#include <set>

using namespace std;

const char* DB_FILE = "database.dat";
const char* INDEX_FILE = "index.dat";
const int MAX_KEY_LEN = 65;

struct Record {
    char key[MAX_KEY_LEN];
    int value;
    bool deleted;
};

class FileDatabase {
private:
    fstream file;
    map<string, set<pair<int, streampos>>> index; // key -> set of (value, file_position)
    
    void openFile() {
        file.open(DB_FILE, ios::in | ios::out | ios::binary);
        if (!file.is_open()) {
            // Create file if it doesn't exist
            ofstream newFile(DB_FILE, ios::binary);
            newFile.close();
            file.open(DB_FILE, ios::in | ios::out | ios::binary);
        }
    }
    
    void loadIndex() {
        ifstream idxFile(INDEX_FILE, ios::binary);
        if (idxFile.is_open()) {
            int count;
            idxFile.read((char*)&count, sizeof(count));
            
            for (int i = 0; i < count; i++) {
                int keyLen;
                idxFile.read((char*)&keyLen, sizeof(keyLen));
                
                char key[MAX_KEY_LEN];
                idxFile.read(key, keyLen);
                key[keyLen] = '\0';
                
                int valueCount;
                idxFile.read((char*)&valueCount, sizeof(valueCount));
                
                for (int j = 0; j < valueCount; j++) {
                    int value;
                    streampos pos;
                    idxFile.read((char*)&value, sizeof(value));
                    idxFile.read((char*)&pos, sizeof(pos));
                    index[string(key)].insert(make_pair(value, pos));
                }
            }
            idxFile.close();
        }
    }
    
    void saveIndex() {
        ofstream idxFile(INDEX_FILE, ios::binary | ios::trunc);
        
        int count = index.size();
        idxFile.write((char*)&count, sizeof(count));
        
        for (auto& pair : index) {
            int keyLen = pair.first.length();
            idxFile.write((char*)&keyLen, sizeof(keyLen));
            idxFile.write(pair.first.c_str(), keyLen);
            
            int valueCount = pair.second.size();
            idxFile.write((char*)&valueCount, sizeof(valueCount));
            
            for (auto& vp : pair.second) {
                idxFile.write((char*)&vp.first, sizeof(vp.first));
                idxFile.write((char*)&vp.second, sizeof(vp.second));
            }
        }
        idxFile.close();
    }
    
public:
    FileDatabase() {
        openFile();
        loadIndex();
    }
    
    ~FileDatabase() {
        saveIndex();
        if (file.is_open()) {
            file.close();
        }
    }
    
    void insert(const string& index_key, int value) {
        // Check if record already exists in index
        auto it = index.find(index_key);
        if (it != index.end()) {
            for (auto& vp : it->second) {
                if (vp.first == value) {
                    return; // Already exists
                }
            }
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
        index[index_key].insert(make_pair(value, pos));
    }
    
    void remove(const string& index_key, int value) {
        auto it = index.find(index_key);
        if (it == index.end()) return;
        
        streampos pos_to_delete = -1;
        for (auto& vp : it->second) {
            if (vp.first == value) {
                pos_to_delete = vp.second;
                break;
            }
        }
        
        if (pos_to_delete != (streampos)-1) {
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
            it->second.erase(make_pair(value, pos_to_delete));
            if (it->second.empty()) {
                index.erase(it);
            }
        }
    }
    
    void find(const string& index_key) {
        auto it = index.find(index_key);
        if (it == index.end() || it->second.empty()) {
            cout << "null" << endl;
        } else {
            bool first = true;
            for (auto& vp : it->second) {
                if (!first) cout << " ";
                cout << vp.first;
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
