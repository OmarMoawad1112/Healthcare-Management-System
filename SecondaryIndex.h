#ifndef HEALTHCAREMANAGEMENTSYSTEM_SECONDARYINDEX_H
#define HEALTHCAREMANAGEMENTSYSTEM_SECONDARYINDEX_H

#include <bits/stdc++.h>

using namespace std;

// Class representing a single node in the primary key list (linked list for secondary index)
class PrimaryKeyNode {
public:
    string primaryKey; // The primary key (e.g., ID)
    string nextIndex;  // The next node in the linked list, stored as a string to represent index

    // Constructor to initialize primaryKey and nextIndex
    PrimaryKeyNode(const string& pk, const string& next) : primaryKey(pk), nextIndex(next) {}
};

class SecondaryIndex {
private:
    string secondaryIndexFileName;       // Name of the secondary index file
    string labelIdListFileName;          // Name of the label ID list file
    map<string, int> secondaryIndexMap;  // Maps secondary key to the index of the head of the linked list
    vector<PrimaryKeyNode> primaryKeyList; // List of PrimaryKeyNodes representing the linked list

public:
    // Get the index of a free label for adding a new PrimaryKeyNode
    int getFreeLabelIndex() {
        for (int index = 0; index < primaryKeyList.size(); ++index) {
            if (primaryKeyList[index].nextIndex == "##") {
                return index;  // Return the first free index (marked as "##")
            }
        }
        primaryKeyList.emplace_back("##", "##");  // If no free label found, add a new one
        return primaryKeyList.size() - 1; // Return the new index
    }

    // Release a label ID to mark it as free
    void releaseLabelId(int index) {
        if (index >= 0 && index < primaryKeyList.size()) {
            primaryKeyList[index].primaryKey = "##";
            primaryKeyList[index].nextIndex = "##";  // Mark it as free
        }
    }

    // Set the filenames for the secondary index and label ID list, and load the data
    void setSecondaryIndexAndLabelIdListFileNames(const string& secondaryIndex, const string& labelIdFileName) {
        this->secondaryIndexFileName = secondaryIndex;
        this->labelIdListFileName = labelIdFileName;
        loadSecondaryIndexAndLabelIdList();  // Load the secondary index and label list data
    }

    // Load secondary index and label list data from files
    void loadSecondaryIndexAndLabelIdList() {
        // Load Secondary Index (secondary key -> head pointer)
        ifstream secFile(secondaryIndexFileName);
        if (!secFile.is_open()) {
            cerr << "Error opening file: " << secondaryIndexFileName << "\n";
            return;
        }

        string line;
        while (getline(secFile, line)) {
            istringstream recordStream(line);
            string secondaryKey, headIndex;
            getline(recordStream, secondaryKey, '|');  // Parse secondary key
            getline(recordStream, headIndex, '|');  // Parse head pointer (index)
            secondaryIndexMap[secondaryKey] = stoi(headIndex);  // Store the head index for the secondary key
        }
        secFile.close();

        // Load Label Id List (linked list of primary keys)
        ifstream labelFile(labelIdListFileName);
        if (!labelFile.is_open()) {
            cerr << "Error opening file: " << labelIdListFileName << "\n";
            return;
        }

        primaryKeyList.clear();  // Clear the list to prepare for loading new data

        string recNoStr, id, nextPtrStr;
        while (getline(labelFile, line)) {
            istringstream recordStream(line);
            getline(recordStream, recNoStr, '|');  // Extract recNo (record number)
            getline(recordStream, id, ',');       // Extract ID (primary key)
            getline(recordStream, nextPtrStr);    // Extract next pointer (index)

            primaryKeyList.emplace_back(id, nextPtrStr);  // Add the node to the linked list
        }
        labelFile.close();
    }

    // Update secondary index and label ID list in their respective files
    void updateSecondaryIndexAndLabelIdList() {
        // Update Secondary Index (secondary key -> head pointer)
        ofstream secFile(secondaryIndexFileName);
        if (!secFile.is_open()) {
            cerr << "Error opening file: " << secondaryIndexFileName << "\n";
            return;
        }
        for (const auto &entry : secondaryIndexMap) {
            secFile << entry.first << "|" << setw(2) << setfill('0') << entry.second << '\n';  // Format secondary key and head index
        }
        secFile.close();

        // Update Label Id List (linked list of primary keys and next pointers)
        ofstream labelFile(labelIdListFileName);
        if (!labelFile.is_open()) {
            cerr << "Error opening file: " << labelIdListFileName << "\n";
            return;
        }
        int recNo = 0;
        for (const auto &node : primaryKeyList) {
            // Format record number, primary key, and next pointer for writing to file
            labelFile << setw(2) << setfill('0') << recNo << "|"
                      << setw(2) << setfill('0') << node.primaryKey << ","
                      << setw(2) << setfill('0') << node.nextIndex << '\n';
            recNo++;
        }
        labelFile.close();
    }

    // Add a primary key to a secondary index node (linked list of primary keys)
    void addPrimaryKeyToSecondaryNode(const string &secondaryKey, const string &primaryKey) {
        int freeLabelId = getFreeLabelIndex();  // Get a free label ID for the new node
        if (secondaryIndexMap.find(secondaryKey) == secondaryIndexMap.end()) {
            // If the secondary key doesn't exist, create a new head node for the linked list
            primaryKeyList[freeLabelId] = PrimaryKeyNode(primaryKey, "-1");  // Set next pointer to -1
            secondaryIndexMap[secondaryKey] = freeLabelId;  // Set head pointer to the new node
        } else {
            // If the secondary key exists, add to the linked list
            int currentIndex = secondaryIndexMap[secondaryKey];
            if (currentIndex == -1) {
                secondaryIndexMap[secondaryKey] = freeLabelId;  // Set head to the new node if the list is empty
            } else {
                while (primaryKeyList[currentIndex].nextIndex != "-1") {
                    currentIndex = stoi(primaryKeyList[currentIndex].nextIndex);  // Traverse to the last node
                }
                primaryKeyList[currentIndex].nextIndex = to_string(freeLabelId);  // Update the last node's next pointer
            }
            primaryKeyList[freeLabelId] = PrimaryKeyNode(primaryKey, "-1");  // Set next pointer to -1 for the new node
        }
        updateSecondaryIndexAndLabelIdList();  // Update the index and label ID list files
    }

    // Remove a primary key from a secondary index node (linked list of primary keys)
    void removePrimaryKeyFromSecondaryNode(const string &secondaryKey, const string &primaryKey) {
        if (secondaryIndexMap.find(secondaryKey) == secondaryIndexMap.end()) {
            cerr << "Error: Secondary key not found.\n";
            return;
        }

        string* prevPtr = &primaryKeyList[secondaryIndexMap[secondaryKey]].nextIndex;
        int currentIndex = secondaryIndexMap[secondaryKey];

        while (currentIndex != -1) {
            if (primaryKeyList[currentIndex].primaryKey == primaryKey) {
                if (currentIndex == secondaryIndexMap[secondaryKey]) {
                    secondaryIndexMap[secondaryKey] = stoi(primaryKeyList[currentIndex].nextIndex);  // Update head pointer
                }
                *prevPtr = primaryKeyList[currentIndex].nextIndex;  // Update the previous node's next pointer
                releaseLabelId(currentIndex);  // Release the label ID of the removed node
                break;
            }
            prevPtr = &primaryKeyList[currentIndex].nextIndex;  // Move to the next node
            currentIndex = stoi(primaryKeyList[currentIndex].nextIndex);
        }

        if (currentIndex == -1) {
            cerr << "Error: Primary key not found.\n";
        }

        updateSecondaryIndexAndLabelIdList();  // Update the index and label ID list files
    }

    // Get all primary keys associated with a secondary key
    vector<string> getPrimaryKeysBySecondaryKey(const string &secondaryKey) {
        vector<string> primaryKeys;
        int index = secondaryIndexMap[secondaryKey];
        while (index != -1) {
            primaryKeys.push_back(primaryKeyList[index].primaryKey);  // Add primary key to the list
            index = stoi(primaryKeyList[index].nextIndex);  // Move to the next node
        }
        return primaryKeys;
    }
};

#endif //HEALTHCAREMANAGEMENTSYSTEM_SECONDARYINDEX_H
