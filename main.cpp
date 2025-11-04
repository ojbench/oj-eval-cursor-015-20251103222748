#include <iostream>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <vector>
#include <set>

using namespace std;

const char* DB_FILE = "database.dat";
const int MAX_KEY_LEN = 65;

struct Record {
    char key[MAX_KEY_LEN];
    int value;
    bool deleted;
};

class FileDatabase {
private:
    fstream file;
    
    void openFile() {
        file.open(DB_FILE, ios::in | ios::out | ios::binary);
        if (!file.is_open()) {
            // Create file if it doesn't exist
            ofstream newFile(DB_FILE, ios::binary);
            newFile.close();
            file.open(DB_FILE, ios::in | ios::out | ios::binary);
        }
    }
    
public:
    FileDatabase() {
        openFile();
    }
    
    ~FileDatabase() {
        if (file.is_open()) {
            file.close();
        }
    }
    
    void insert(const string& index, int value) {
        // Check if record already exists
        file.clear();
        file.seekg(0, ios::beg);
        
        Record rec;
        bool found = false;
        
        while (file.read((char*)&rec, sizeof(Record))) {
            if (!rec.deleted && strcmp(rec.key, index.c_str()) == 0 && rec.value == value) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            // Add new record at the end
            file.clear();
            file.seekp(0, ios::end);
            
            Record newRec;
            strncpy(newRec.key, index.c_str(), MAX_KEY_LEN - 1);
            newRec.key[MAX_KEY_LEN - 1] = '\0';
            newRec.value = value;
            newRec.deleted = false;
            
            file.write((char*)&newRec, sizeof(Record));
            file.flush();
        }
    }
    
    void remove(const string& index, int value) {
        file.clear();
        file.seekg(0, ios::beg);
        
        Record rec;
        streampos pos;
        
        while (file.read((char*)&rec, sizeof(Record))) {
            if (!rec.deleted && strcmp(rec.key, index.c_str()) == 0 && rec.value == value) {
                // Mark as deleted
                pos = file.tellg();
                pos -= sizeof(Record);
                
                rec.deleted = true;
                
                file.seekp(pos);
                file.write((char*)&rec, sizeof(Record));
                file.flush();
                break;
            }
        }
    }
    
    void find(const string& index) {
        file.clear();
        file.seekg(0, ios::beg);
        
        vector<int> values;
        Record rec;
        
        while (file.read((char*)&rec, sizeof(Record))) {
            if (!rec.deleted && strcmp(rec.key, index.c_str()) == 0) {
                values.push_back(rec.value);
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
