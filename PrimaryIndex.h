#ifndef HEALTHCAREMANAGEMENTSYSTEM_PRIMARYINDEX_H
#define HEALTHCAREMANAGEMENTSYSTEM_PRIMARYINDEX_H

#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>

using namespace std;

// Helper function to check if a file is empty
static bool isFileEmpty(const string &filename) {
    ifstream file(filename, ios::in);
    return file.peek() == ifstream::traits_type::eof();  // Check if the file is empty
}

// Class representing a single node in the primary index
class PrimaryIndexNode {
public:
    string primaryKey; // The primary key (e.g., ID)
    int offset;        // The offset to the record's position in the data file

    // Constructor to initialize the primary key and its offset
    PrimaryIndexNode(const string &primaryKey, int offset) {
        this->primaryKey = primaryKey;
        this->offset = offset;
    }

    // Overload the less-than operator to allow sorting of primary index nodes
    bool operator<(const PrimaryIndexNode &other) const {
        return (this->primaryKey < other.primaryKey);
    }
};

// Class representing the primary index for the system
class PrimaryIndex {
    string primaryIndexFileName;       // Name of the primary index file
    vector<PrimaryIndexNode> primaryIndex; // Vector to store primary index nodes

public:
    // Set the primary index file name and load the index into memory
    void setPrimaryIndexFileName(const string& fileName) {
        this->primaryIndexFileName = fileName;
        loadPrimaryIndexInMemory();
    }

    // Generate a new unique ID based on the last primary key in the index
    string getNewId() {
        if (primaryIndex.empty()) {
            return "01";  // Start with a two-digit ID
        } else {
            int newId = stoi(primaryIndex.back().primaryKey) + 1;
            return (newId < 10) ? "0" + to_string(newId) : to_string(newId); // Ensure two-digit IDs
        }
    }

    // Get all primary index nodes
    vector<PrimaryIndexNode> getPrimaryIndexNodes() const {
        return primaryIndex;
    }

    // Load the primary index from a file into memory
    void loadPrimaryIndexInMemory() {
        ifstream file(primaryIndexFileName, ios::in);
        if (!file.is_open()) {
            cerr << "Error opening file: PrimaryIndex.txt\n";
            return;
        }
        if (isFileEmpty(primaryIndexFileName)) {
            return;  // No data to load if the file is empty
        }

        string line;
        while (getline(file, line)) {
            istringstream recordStream(line);
            string primaryKey, offset;

            // Read the primary key and offset from the file
            getline(recordStream, primaryKey, '|');
            getline(recordStream, offset, '|');

            // Add the index node with the read primary key and offset
            primaryIndex.emplace_back(primaryKey, stoi(offset));
        }

        file.close();
    }

    // Update the primary index file with the in-memory data
    void updatePrimaryIndexFile() {
        fstream outFile(primaryIndexFileName, ios::out | ios::trunc);
        if (!outFile.is_open()) {
            cerr << "Error opening file: PrimaryIndex.txt\n";
            return;
        }
        for (const auto &ele : primaryIndex) {
            outFile << ele.primaryKey << '|' << ele.offset << '\n'; // Write each primary key and its offset
        }
        outFile.close();
    }

    // Add a new primary key and offset to the index and update the file
    void addPrimaryNode(const string &primaryKey, int offset) {
        primaryIndex.emplace_back(primaryKey, offset);  // Add the new node
        sortPrimaryIndex();  // Sort the index nodes after addition
        updatePrimaryIndexFile(); // Write the updated index to the file
    }

    // Remove a primary key node from the index and update the file
    void removePrimaryNode(const string &primaryKey) {
        // Perform binary search to find the node
        int left = 0, right = primaryIndex.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (primaryIndex[mid].primaryKey == primaryKey) {
                // Node found, remove it
                primaryIndex.erase(primaryIndex.begin() + mid);
                sortPrimaryIndex();  // Re-sort the index after removal
                updatePrimaryIndexFile();  // Update the index file
                return;
            } else if (primaryIndex[mid].primaryKey < primaryKey) {
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        cerr << "Error: Primary key not found.\n";  // If the key is not found
    }

    // Sort the primary index by primary key
    void sortPrimaryIndex() {
        sort(primaryIndex.begin(), primaryIndex.end());  // Sort using the overloaded operator<
    }

    // Perform binary search to find the offset of a given primary key
    int binarySearchPrimaryIndex(const string &primaryKey) {
        int left = 0;
        int right = primaryIndex.size() - 1;
        while (left <= right) {
            int mid = left + (right - left) / 2;
            if (primaryIndex[mid].primaryKey == primaryKey) {
                return primaryIndex[mid].offset;  // Return the offset if the key is found
            } else if (primaryKey < primaryIndex[mid].primaryKey) {
                right = mid - 1;
            } else {
                left = mid + 1;
            }
        }
        return -1;  // Return -1 if the key is not found
    }
};

#endif //HEALTHCAREMANAGEMENTSYSTEM_PRIMARYINDEX_H
