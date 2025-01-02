#ifndef HEALTHCAREMANAGEMENTSYSTEM_DOCTORMANAGEMENTSYSTEM_H
#define HEALTHCAREMANAGEMENTSYSTEM_DOCTORMANAGEMENTSYSTEM_H

#include "PrimaryIndex.h"
#include "SecondaryIndex.h"
#include "AvailList.h"

using namespace std;


// Class representing a doctor in the healthcare management system
class Doctor {
public:
    // Default constructor initializing fields to empty strings
    Doctor() {
        id = name = address = "";
    }

    // Attributes of the Doctor class
    string id;    // Primary Key - Unique identifier for a doctor
    string name;  // Secondary Key - Doctor's name, can be used for searching/sorting
    string address; // Doctor's address

    // Parameterized constructor to initialize a Doctor object with specific details
    Doctor(const string &id, const string &name, const string &address)
            : id(id), name(name), address(address) {}
};


class DoctorManagementSystem {
private:
    // Objects for managing primary and secondary indices and an availability list
    PrimaryIndex doctorPrimaryIndex;
    SecondaryIndex doctorSecondaryIndex;
    AvailList doctorAvailList;

public:
    // Constructor to set file names for indices and availability list
    DoctorManagementSystem() {
        doctorPrimaryIndex.setPrimaryIndexFileName("DoctorPrimaryIndex.txt");
        doctorSecondaryIndex.setSecondaryIndexAndLabelIdListFileNames("DoctorSecondaryIndex.txt",
                                                                      "DoctorLabelIdList.txt");
        doctorAvailList.setAvailListFileName("DoctorAvailList.txt");
    }

    // Getter for the primary index
    PrimaryIndex &getDoctorPrimaryIndex() {
        return doctorPrimaryIndex;
    }

    // Function to add a new doctor record
    void addDoctor(Doctor &doctor) {
        // Generate a new unique ID for the doctor
        doctor.id = doctorPrimaryIndex.getNewId();

        fstream file("doctors.txt", ios::in | ios::out);
        if (!file.is_open()) {
            cerr << "Error opening file: doctors.txt" << endl;
            return;
        }

        // Calculate the length of the record to be stored
        int lengthIndicator = static_cast<int>(doctor.id.size()) +
                              static_cast<int>(doctor.name.size()) +
                              static_cast<int>(doctor.address.size()) + 4;

        // Find the best fit for the record in the availability list
        AvailListNode *node = doctorAvailList.bestFit(lengthIndicator);

        string newRecord = "";
        int offset;

        if (node != nullptr) {
            // Reuse a deleted record's space if a suitable node is found
            file.seekp(node->offset, ios::beg);
            file.put(' '); // Mark the record as active

            file.seekp(3, ios::cur);
            newRecord += "|" + doctor.id + "|" + doctor.name + "|" + doctor.address + "|";

            // Pad with '-' if the reused space is larger than the new record
            int padding = node->size - static_cast<int>(newRecord.size());
            if (padding >= 0) {
                newRecord.append(padding, '-');
            }

            newRecord += '\n';
            file.write(newRecord.c_str(), node->size);
            offset = node->offset;

            doctorAvailList.remove(node); // Remove the node from the availability list
        } else {
            // Append a new record at the end if no suitable space is found
            newRecord += " |";
            if (lengthIndicator < 10) {
                newRecord += '0';
            }
            newRecord += to_string(lengthIndicator) + "|" + doctor.id + "|" + doctor.name + "|" + doctor.address + "|\n";

            file.seekp(0, ios::end);
            offset = static_cast<int>(file.tellp());
            file.write(newRecord.c_str(), newRecord.size());
        }

        cout << "Doctor " << doctor.name << " is added with ID " << stoi(doctor.id) << endl;

        file.close();

        // Update the indices with the new record information
        doctorPrimaryIndex.addPrimaryNode(doctor.id, offset);
        doctorSecondaryIndex.addPrimaryKeyToSecondaryNode(doctor.name, doctor.id);
    }

    // Function to update a doctor's name
    void updateDoctorName(const string &id, string &newName) {
        // Find the doctor's record offset in the primary index
        int offset = doctorPrimaryIndex.binarySearchPrimaryIndex(id);

        if (offset == -1) {
            cerr << "Error: Doctor ID not found in primary index.\n";
            return;
        }
        fstream doctorFile("doctors.txt", ios::in | ios::out);
        if (!doctorFile.is_open()) {
            cerr << "Error: Could not open doctors.txt file.\n";
            return;
        }

        // Move to the record's offset and read its content
        doctorFile.seekg(offset, ios::beg);
        string line;
        getline(doctorFile, line);

        // Parse the record
        istringstream recordStream(line);
        string status, recordLen, record_id, name, address;
        getline(recordStream, status, '|');
        getline(recordStream, recordLen, '|');
        getline(recordStream, record_id, '|');
        getline(recordStream, name, '|');
        getline(recordStream, address, '|');

        // Calculate the new record size
        int newSize = newName.size() + record_id.size() + address.size() + 4;

        if (newSize <= stoi(recordLen)) {
            // Update the record directly if it fits in the same space
            doctorSecondaryIndex.removePrimaryKeyFromSecondaryNode(name, id);
            doctorSecondaryIndex.addPrimaryKeyToSecondaryNode(newName, id);

            doctorFile.seekp(offset + 8, ios::beg);
            doctorFile << newName << '|' << address << '|';

            // Add padding if there's excess space
            int excess = stoi(recordLen) - newSize;
            for (int i = 0; i < excess; ++i) {
                doctorFile << '-';
            }
        } else {
            // Delete the current record and add a new one if space is insufficient
            deleteDoctor(record_id);
            Doctor doctor;
            doctor.id = doctorPrimaryIndex.getNewId();
            doctor.name = newName;
            doctor.address = address;
            addDoctor(doctor);
        }
        cout << "Doctor's name updated successfully.\n";

        doctorFile.close();
    }

    // Function to delete a doctor's record
    void deleteDoctor(const string &id) {
        // Find the record's offset in the primary index
        int offset = doctorPrimaryIndex.binarySearchPrimaryIndex(id);
        if (offset == -1) {
            cout << "Doctor with ID " << id << " not found.\n";
            return;
        }

        fstream doctorFile("doctors.txt", ios::in | ios::out);
        if (!doctorFile.is_open()) {
            cerr << "Error opening file: doctors.txt\n";
            return;
        }

        // Mark the record as deleted by replacing its status byte
        doctorFile.seekp(offset, ios::beg);
        doctorFile.put('*');

        doctorFile.seekg(-1, ios::cur);
        string line;
        getline(doctorFile, line);

        // Parse the record
        istringstream recordStream(line);
        string status, recordLen, record_id, name, address;
        getline(recordStream, status, '|');
        getline(recordStream, recordLen, '|');
        getline(recordStream, record_id, '|');
        getline(recordStream, name, '|');
        getline(recordStream, address, '|');

        // Get the record's length and add it to the availability list
        int lengthIndicator = stoi(recordLen);
        AvailListNode *newNode = new AvailListNode(offset, lengthIndicator);
        doctorAvailList.insert(newNode);

        // Remove the doctor from the indices
        doctorPrimaryIndex.removePrimaryNode(id);
        doctorSecondaryIndex.removePrimaryKeyFromSecondaryNode(name, id);

        cout << "Doctor with ID " << stoi(id) << " has been marked as deleted.\n";

        doctorFile.close();
    }

    // Function to search for doctors by their name using the secondary index
    vector<string> searchDoctorsByName(const string &name) {
        // Retrieve a list of doctor IDs associated with the given name
        vector<string> doctorIds = doctorSecondaryIndex.getPrimaryKeysBySecondaryKey(name);
        return doctorIds;
    }

    // Function to print a doctor's details by their ID
    void printDoctorById(const string &id, int choice) {
        // Find the record offset for the given doctor ID using the primary index
        int offset = doctorPrimaryIndex.binarySearchPrimaryIndex(id);
        if (offset == -1) {
            cout << "Doctor not found. The ID \"" << id << "\" is invalid.\n";
            return;
        }

        // Open the doctors' file to retrieve the record
        fstream file("doctors.txt", ios::in);
        if (!file.is_open()) {
            cout << "Error opening file.\n";
            return;
        }

        // Move to the record's offset and read the line
        file.seekg(offset, ios::beg);
        string line;
        getline(file, line);

        if (line.empty()) {
            cout << "Error: Empty record at offset " << offset << ".\n";
            return;
        }

        // Parse the record into its components
        istringstream recordStream(line);
        string status, length, record_id, name, address;
        getline(recordStream, status, '|');
        getline(recordStream, length, '|');
        getline(recordStream, record_id, '|');
        getline(recordStream, name, '|');
        getline(recordStream, address, '|');

        file.close();

        // Remove padding characters ('-') from the name
        string temp;
        for (char ch : name) {
            if (ch != '-') temp += ch;
        }
        name = temp;

        // Print the requested information based on the choice parameter
        if (choice == 0) {
            cout << "ID: " << stoi(record_id) << " | Name: " << name << " | Address: " << address << '\n';
        } else if (choice == 1) {
            cout << "ID: " << stoi(record_id) << '\n';
        } else if (choice == 2) {
            cout << "Name: " << name << '\n';
        } else if (choice == 3) {
            cout << "Address: " << address << '\n';
        } else {
            cout << "Doctor's info:\n"
                 << "  ID: " << stoi(record_id) << '\n'
                 << "  Name: " << name << '\n'
                 << "  Address: " << address << '\n';
        }
    }

    // Function to print doctors whose address matches a given value
    void printDoctorByAddress(const string &address, int choice) {
        // Retrieve all primary index nodes
        const vector<PrimaryIndexNode>& primaryIndexNodes = doctorPrimaryIndex.getPrimaryIndexNodes();

        // Open the doctors' file for reading
        ifstream doctorsFile("doctors.txt", ios::in);
        if (!doctorsFile.is_open()) {
            cerr << "Error opening doctor file.\n";
            return;
        }

        // Iterate through all primary index nodes to find matching records
        for (const PrimaryIndexNode& node : primaryIndexNodes) {
            int offset = node.offset;

            // Move to the offset and read the record
            doctorsFile.seekg(offset, ios::beg);
            string line;
            getline(doctorsFile, line);

            // Skip deleted records or empty lines
            if (line.empty() || line[0] == '*') continue;

            // Parse the record into its components
            istringstream recordStream(line);
            string status, len, id, name, recAddress;
            getline(recordStream, status, '|');
            getline(recordStream, len, '|');
            getline(recordStream, id, '|');
            getline(recordStream, name, '|');
            getline(recordStream, recAddress, '|');

            // If the record's address matches the given address, print the information
            if (recAddress == address) {
                if (choice == 0) {
                    cout << "ID: " << stoi(id) << " | Name: " << name << " | Address: " << address << '\n';
                } else if (choice == 1) {
                    cout << "ID: " << stoi(id) << '\n';
                } else if (choice == 2) {
                    cout << "Name: " << name << '\n';
                } else if (choice == 3) {
                    cout << "Address: " << address << '\n';
                } else {
                    cout << "Doctor's info:\n"
                         << "  ID: " << stoi(id) << '\n'
                         << "  Name: " << name << '\n'
                         << "  Address: " << address << '\n';
                }
            }
        }

        doctorsFile.close();
    }

    // Function to print all doctors' records
    void printAllDoctors(int choice) {
        // Open the doctors' file and the primary index file
        ifstream doctors("doctors.txt", ios::in);
        ifstream primaryIndexFile("DoctorPrimaryIndex.txt", ios::in);
        if (!doctors) {
            cerr << "Error opening doctor file.\n";
            return;
        }
        if (!primaryIndexFile) {
            cerr << "Error opening primary index file.\n";
            return;
        }

        // Read each record from the primary index and print corresponding details
        string line1, status, len, id, offset, name, address;
        while (getline(primaryIndexFile, line1)) {
            istringstream recordStream1(line1);
            getline(recordStream1, id, '|');
            getline(recordStream1, offset, '|');

            // Read the record from the doctors' file
            doctors.seekg(stoi(offset), ios::beg);
            string line2;
            getline(doctors, line2);
            istringstream recordStream2(line2);

            // Parse the record
            getline(recordStream2, status, '|');
            getline(recordStream2, len, '|');
            getline(recordStream2, id, '|');
            getline(recordStream2, name, '|');
            getline(recordStream2, address, '|');

            // Print the requested information based on the choice parameter
            if (choice == 0) {
                cout << "ID: " << stoi(id) << " | Name: " << name << " | Address: " << address << '\n';
            } else if (choice == 1) {
                cout << "ID: " << stoi(id) << '\n';
            } else if (choice == 2) {
                cout << "Name: " << name << '\n';
            } else if (choice == 3) {
                cout << "Address: " << address << '\n';
            }
        }

        doctors.close();
        primaryIndexFile.close();
    }

};


#endif //HEALTHCAREMANAGEMENTSYSTEM_DOCTORMANAGEMENTSYSTEM_H
