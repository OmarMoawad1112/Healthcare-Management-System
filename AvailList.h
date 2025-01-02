#ifndef HEALTHCAREMANAGEMENTSYSTEM_AVAILLIST_H
#define HEALTHCAREMANAGEMENTSYSTEM_AVAILLIST_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cctype>
#include <algorithm>

using namespace std;

// Class representing a node in the available memory list
class AvailListNode {
public:
    int offset;  // Offset of the available block
    int size;    // Size of the available block
    AvailListNode *next;  // Pointer to the next node in the list

    // Constructor to initialize the node with offset, size, and next as nullptr
    AvailListNode(int offset, int size) : offset(offset), size(size), next(nullptr) {}
};

// Class representing the list of available memory blocks
class AvailList {
private:
    string availListFileName;  // Filename of the available memory list file
    AvailListNode *header;     // Head node of the linked list

public:
    // Constructor initializes an empty list (header is nullptr)
    AvailList() : header(nullptr) {}

    // Set the filename for the available list and load the data into memory
    void setAvailListFileName(const string& fileName) {
        this->availListFileName = fileName;
        loadAvailListInMemory();
    }

    // Insert a new node in the available list in sorted order by size
    void insert(AvailListNode *newNode) {
        if (header == nullptr) { // If the list is empty, set the new node as the head
            header = newNode;
        } else { // Insert in sorted order
            AvailListNode *prev = nullptr;
            AvailListNode *curr = header;

            // Traverse to find the correct position to insert the new node
            while (curr != nullptr && curr->size < newNode->size) {
                prev = curr;
                curr = curr->next;
            }

            // If inserting at the beginning
            if (prev == nullptr) {
                newNode->next = header;
                header = newNode;
            } else { // Insert in the middle or end
                prev->next = newNode;
                newNode->next = curr;
            }
        }
        updateAvailListFile();  // Update the file after insertion
    }

    // Remove a node from the available list
    void remove(AvailListNode *nodeToRemove) {
        if (header == nullptr || nodeToRemove == nullptr) {
            return; // List is empty or invalid node
        }

        // Case 1: Node to remove is the head of the list
        if (header == nodeToRemove) {
            header = header->next;
            delete nodeToRemove;
            updateAvailListFile();  // Update the file after removal
            return;
        }

        // Case 2: Node to remove is not the head
        AvailListNode *prev = nullptr;
        AvailListNode *curr = header;

        // Traverse the list to find the node to remove
        while (curr != nullptr && curr != nodeToRemove) {
            prev = curr;
            curr = curr->next;
        }

        // If the node is found, remove it
        if (curr == nodeToRemove) {
            prev->next = curr->next;
            delete curr;
            updateAvailListFile();  // Update the file after removal
        }
    }

    // Find the best fit node for a given size (a node with a size >= newSize)
    AvailListNode *bestFit(int newSize) {
        AvailListNode *curr = header;
        while (curr != nullptr && curr->size < newSize) {
            curr = curr->next;  // Move to the next node if current size is smaller
        }
        return curr;  // Return the first node that fits
    }

    // Load the available list data from the file into memory
    void loadAvailListInMemory() {
        fstream availListFile(availListFileName, ios::in);

        if (!availListFile.is_open()) {
            cerr << "Error opening file: " << availListFileName << endl;
            return;
        }
        if (isFileEmpty(availListFileName)) {
            return; // Return early if the file is empty
        }

        string line;
        while (getline(availListFile, line)) {
            istringstream stream(line);
            string offset, size;
            getline(stream, offset, '|');
            getline(stream, size, '|');

            AvailListNode *newNode = new AvailListNode(stoi(offset), stoi(size));
            insert(newNode);  // Insert the node into the list
        }
    }

    // Update the available list file with the current in-memory data
    void updateAvailListFile() {
        // Open the file in output mode (overwrites the file)
        fstream availFile(availListFileName, ios::out);

        if (!availFile.is_open()) {
            cerr << "Error opening file: " << availListFileName << endl;
            return;
        }

        AvailListNode *curr = header;

        // Traverse through the list and write each node's data to the file
        while (curr != nullptr) {
            availFile << curr->offset << "|" << curr->size << '\n';  // Write offset and size
            curr = curr->next;
        }
        availFile.close();  // Close the file after writing
    }

    // Destructor to clean up the allocated memory
    ~AvailList() {
        while (header) {
            AvailListNode *temp = header;
            header = header->next;
            delete temp;  // Delete each node
        }
    }
};

#endif //HEALTHCAREMANAGEMENTSYSTEM_AVAILLIST_H
