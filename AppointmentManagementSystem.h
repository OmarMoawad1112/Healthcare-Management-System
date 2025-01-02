#ifndef HEALTHCAREMANAGEMENTSYSTEM_APPOINTMENTMANAGEMENTSYSTEM_H
#define HEALTHCAREMANAGEMENTSYSTEM_APPOINTMENTMANAGEMENTSYSTEM_H

#include "DoctorManagementSystem.h"
#include "AvailList.h"
#include "PrimaryIndex.h"
#include "SecondaryIndex.h"

using namespace std;

// Class representing an appointment in the healthcare management system
class Appointment {
public:
    // Default constructor to initialize member variables
    Appointment() {
        id = "";     // Initialize the appointment ID to an empty string
        date = "";   // Initialize the date to an empty string
        doctorID = ""; // Initialize the doctor ID to an empty string
    }

    // Attributes of the Appointment class
    string id;       // Primary Key - Unique identifier for an appointment
    string date;     // Secondary Key - Date of the appointment, used for searching/sorting
    string doctorID; // Foreign Key - Links this appointment to a specific doctor
};


// The `AppointmentManagementSystem` class is responsible for managing appointments,
// including adding, updating, deleting, and searching for appointments using primary
// and secondary indexing techniques.
class AppointmentManagementSystem {
private:
    PrimaryIndex &doctorPrimaryIndex;  // Reference to shared doctor primary index.
    PrimaryIndex appointmentPrimaryIndex;  // Manages primary index for appointment IDs.
    AvailList appointmentAvailList;        // Manages available space in the file.
    SecondaryIndex appointmentSecondaryIndex; // Manages secondary index for appointments.

public:
    // Constructor: Initializes file names for indexes and the availability list.
    AppointmentManagementSystem(PrimaryIndex &sharedDoctorPrimaryIndex)
            : doctorPrimaryIndex(sharedDoctorPrimaryIndex) {
        // Initialize the file names for the primary index, avail list, and secondary index
        appointmentPrimaryIndex.setPrimaryIndexFileName("AppointmentPrimaryIndex.txt");
        appointmentAvailList.setAvailListFileName("AppointmentAvailList.txt");
        appointmentSecondaryIndex.setSecondaryIndexAndLabelIdListFileNames(
                "AppointmentSecondaryIndex.txt", "AppointmentLabelIdList.txt");
    }

    // Provides access to the primary index for appointments.
    PrimaryIndex &getAppointmentPrimaryIndex() {
        return appointmentPrimaryIndex;
    }

    // Provides access to the secondary index for appointments.
    SecondaryIndex &getAppointmentSecondaryIndex() {
        return appointmentSecondaryIndex;
    }

    // Adds a new appointment to the system.
    void addAppointment(Appointment &appointment) {
        // Validate that the doctor exists in the doctor index
        if (doctorPrimaryIndex.binarySearchPrimaryIndex(appointment.doctorID) == -1) {
            cout << "Error: Doctor ID " << appointment.doctorID
                 << " does not exist. Cannot add appointment.\n";
            return;
        }

        // Generate a new unique ID for the appointment
        appointment.id = appointmentPrimaryIndex.getNewId();

        // Open the appointments file for reading and writing
        fstream file("appointments.txt", ios::in | ios::out);
        if (!file.is_open()) {
            cerr << "Error: Could not open appointments.txt\n";
            return;
        }

        // Calculate the size of the new appointment record
        int recordSize = appointment.id.size() + appointment.date.size() +
                         appointment.doctorID.size() + 4;

        // Attempt to find a suitable space in the availability list
        AvailListNode *availableNode = appointmentAvailList.bestFit(recordSize);

        string newRecord;
        int offset; // Offset where the record will be written

        if (availableNode != nullptr) {
            // Write the record into the found space
            file.seekp(availableNode->offset, ios::beg);
            file.put(' '); // Mark the record as active
            file.seekp(3, ios::cur); // Skip status and size

            newRecord += "|" + appointment.id + "|" + appointment.date + "|" + appointment.doctorID + "|";
            int padding = availableNode->size - newRecord.size();

            if (padding >= 0) {
                newRecord.append(padding, '-'); // Pad with '-' if required
            } else {
                cerr << "Error: Record size exceeds available space.\n";
                file.close();
                return;
            }

            newRecord += '\n';
            file.write(newRecord.c_str(), availableNode->size);
            offset = availableNode->offset;

            // Remove the node from the availability list
            appointmentAvailList.remove(availableNode);
        } else {
            // If no suitable space is available, append the record to the end of the file
            newRecord += " |";
            if (recordSize < 10) {
                newRecord += '0'; // For consistent formatting
            }
            newRecord += to_string(recordSize) + "|" + appointment.id + "|" +
                         appointment.date + "|" + appointment.doctorID + "|\n";

            file.seekp(0, ios::end);
            offset = static_cast<int>(file.tellp());
            file.write(newRecord.c_str(), newRecord.size());
        }

        cout << "Appointment with ID " << stoi(appointment.id) << " has been added.\n";

        file.close();

        // Update indexes
        appointmentPrimaryIndex.addPrimaryNode(appointment.id, offset);
        appointmentSecondaryIndex.addPrimaryKeyToSecondaryNode(appointment.doctorID, appointment.id);
    }

    // Function to update an appointment's date
    void updateAppointmentDate(const string &appointmentID, string &newDate) {
        // Find the appointment's record offset in the primary index
        int offset = appointmentPrimaryIndex.binarySearchPrimaryIndex(appointmentID);

        if (offset == -1) {
            cerr << "Error: Appointment ID not found in primary index.\n";
            return;
        }

        // Open the appointments file for reading and updating
        fstream appointmentFile("appointments.txt", ios::in | ios::out | ios::binary);
        if (!appointmentFile.is_open()) {
            cerr << "Error: Could not open appointments.txt file.\n";
            return;
        }

        // Move to the record's offset and read its content
        appointmentFile.seekg(offset, ios::beg);
        string line;
        getline(appointmentFile, line);

        // Parse the record
        istringstream recordStream(line);
        string status, recordLen, id, oldDate, doctorID;
        getline(recordStream, status, '|');
        getline(recordStream, recordLen, '|');
        getline(recordStream, id, '|');
        getline(recordStream, oldDate, '|');
        getline(recordStream, doctorID, '|');

        // Calculate the new record size
        int newSize = newDate.size() + id.size() + doctorID.size() + 4; // 4 is for the separators

        if (newSize <= stoi(recordLen)) {
            // Update the appointment date directly if it fits in the same space
            appointmentFile.seekp(offset + status.size() + 1 + recordLen.size() + 1 + id.size() + 1, ios::beg);
            appointmentFile << newDate << '|';

            // Add padding if necessary
            int excess = stoi(recordLen) - newSize;
            for (int i = 0; i < excess; ++i) {
                appointmentFile << '-';
            }
        } else {
            // If the new date is too long, delete the old appointment and add a new one
            deleteAppointment(id);  // Delete the existing appointment

            Appointment newAppointment;
            newAppointment.id = appointmentPrimaryIndex.getNewId();
            newAppointment.date = newDate;
            newAppointment.doctorID = doctorID;
            addAppointment(newAppointment);  // Re-add the new appointment

            cout << "Appointment date updated successfully.\n";
            appointmentFile.close();
            return;
        }

        cout << "Appointment date updated successfully.\n";
        appointmentFile.close();
    }

    // Deletes an appointment by marking it as deleted in the file,
    void deleteAppointment(const string &id) {
        // Locate the appointment in the primary index using its ID
        int offset = appointmentPrimaryIndex.binarySearchPrimaryIndex(id);
        if (offset == -1) {
            // If the appointment ID is not found, display an error message and exit
            cout << "Appointment with ID " << id << " not found.\n";
            return;
        }

        // Open the appointments file for reading and writing
        fstream appointmentFile("appointments.txt", ios::in | ios::out);
        if (!appointmentFile.is_open()) {
            cerr << "Error opening file: appointments.txt\n";
            return;
        }

        // Navigate to the location of the record in the file
        appointmentFile.seekp(offset, ios::beg);

        // Mark the record as deleted by writing '*' to the status field
        appointmentFile.put('*');

        // Return to the beginning of the record to read its content
        appointmentFile.seekg(-1, ios::cur);
        string line;
        getline(appointmentFile, line); // Read the full record

        // Parse the record fields
        istringstream recordStream(line);
        string status, recordLen, record_id, date, doctorID;

        getline(recordStream, status, '|');       // Read the status field
        getline(recordStream, recordLen, '|');   // Read the record length
        getline(recordStream, record_id, '|');   // Read the appointment ID
        getline(recordStream, date, '|');        // Read the appointment date
        getline(recordStream, doctorID, '|');    // Read the doctor ID

        // Convert the record length from string to an integer
        int lengthIndicator = stoi(recordLen);

        // Display a confirmation message
        cout << "Appointment with ID " << stoi(id) << " has been marked as deleted.\n";

        // Close the file after marking the record
        appointmentFile.close();

        // Add the space of the deleted record to the availability list
        AvailListNode *newNode = new AvailListNode(offset, lengthIndicator);
        appointmentAvailList.insert(newNode);

        // Remove the appointment from the primary and secondary indexes
        appointmentPrimaryIndex.removePrimaryNode(id);
        appointmentSecondaryIndex.removePrimaryKeyFromSecondaryNode(doctorID, id);
    }

    // Searches for appointments associated with a specific doctor ID
    vector<string> searchAppointmentsByDoctorID(const string &doctorID) {
        // Use the secondary index to find all appointments associated with the doctor ID
        vector<string> appointmentIds = appointmentSecondaryIndex.getPrimaryKeysBySecondaryKey(doctorID);
        return appointmentIds; // Return the list of appointment IDs
    }

    // Prints details of an appointment based on its ID.
    void printAppointmentById(const string &id, int choice) {
        // Locate the appointment using its primary index
        int offset = appointmentPrimaryIndex.binarySearchPrimaryIndex(id);
        if (offset == -1) {
            // If the ID is not found, display an error message and exit
            cout << "Appointment not found. The ID \"" << id << "\" is invalid.\n";
            return;
        }

        // Open the appointments file for reading
        fstream file("appointments.txt", ios::in);
        if (!file.is_open()) {
            cout << "Error opening file: appointments.txt\n";
            return;
        }

        // Navigate to the offset and read the record
        file.seekg(offset, ios::beg);
        string line;
        getline(file, line);  // Read the complete record as a string

        if (line.empty()) {
            // Handle the case where the record at the offset is empty
            cout << "Error: Empty record at offset " << offset << ".\n";
            file.close();
            return;
        }

        // Parse the record into its individual fields
        istringstream recordStream(line);
        string status, length, appointmentID, date, doctorID;

        getline(recordStream, status, '|');       // Read the status field
        getline(recordStream, length, '|');      // Read the record length
        getline(recordStream, appointmentID, '|'); // Read the appointment ID
        getline(recordStream, date, '|');        // Read the date
        getline(recordStream, doctorID, '|');    // Read the doctor ID

        file.close();

        // Remove any padding characters ('-') from the date field
        string tempDate;
        for (char c : date) {
            if (c != '-') {
                tempDate += c;
            }
        }
        date = tempDate;

        // Output the appointment details based on the user's choice
        switch (choice) {
            case 0:  // Print all appointment information
                cout << "Appointment ID: " << stoi(appointmentID)
                     << " | Date: " << date
                     << " | Doctor ID: " << stoi(doctorID) << '\n';
                break;
            case 1:  // Print only the Appointment ID
                cout << "Appointment ID: " << stoi(appointmentID) << '\n';
                break;
            case 2:  // Print only the Date
                cout << "Date: " << date << '\n';
                break;
            case 3:  // Print only the Doctor ID
                cout << "Doctor ID: " << stoi(doctorID) << '\n';
                break;
            default:  // Default to printing all information
                cout << "Appointment Details:\n"
                     << "  ID: " << appointmentID << '\n'
                     << "  Date: " << date << '\n'
                     << "  Doctor ID: " << stoi(doctorID) << '\n';
                break;
        }
    }

    // Prints all appointments matching a specific date.
    void printAppointmentByDate(const string &dateComp, int choice) {
        // Retrieve the primary index nodes in memory
        const vector<PrimaryIndexNode>& primaryIndexNodes = appointmentPrimaryIndex.getPrimaryIndexNodes();

        // Open the appointments file for reading
        ifstream appointments("appointments.txt", ios::in);
        if (!appointments) {
            cerr << "Error opening appointments file.\n";
            return;
        }

        // Loop through each node in the primary index
        for (const auto &node : primaryIndexNodes) {
            // Seek the corresponding record in the appointments file
            appointments.seekg(node.offset, ios::beg);

            string appointmentLine;
            getline(appointments, appointmentLine); // Read the record line
            istringstream recordStream(appointmentLine);

            // Extract appointment details
            string status, len, appId, date, doctorId;
            getline(recordStream, status, '|');   // Read status field
            getline(recordStream, len, '|');     // Read length field
            getline(recordStream, appId, '|');   // Read appointment ID
            getline(recordStream, date, '|');    // Read date field
            getline(recordStream, doctorId, '|'); // Read doctor ID

            // Match and display records that match the specified date
            if (date == dateComp) {
                switch (choice) {
                    case 0:  // Print all appointment details
                        cout << "Appointment ID: " << stoi(appId)
                             << " | Date: " << date
                             << " | Doctor ID: " << stoi(doctorId) << '\n';
                        break;
                    case 1:  // Print only the Appointment ID
                        cout << "Appointment ID: " << stoi(appId) << '\n';
                        break;
                    case 2:  // Print only the Date
                        cout << "Date: " << date << '\n';
                        break;
                    case 3:  // Print only the Doctor ID
                        cout << "Doctor ID: " << stoi(doctorId) << '\n';
                        break;
                    default:  // Default to printing all details
                        cout << "Appointment Details:\n"
                             << "  ID: " << stoi(appId) << '\n'
                             << "  Date: " << date << '\n'
                             << "  Doctor ID: " << stoi(doctorId) << '\n';
                }
            }
        }

        // Close the appointments file
        appointments.close();
    }

    // Prints all appointments stored in the file.
    void printAllAppointments(int choice) {
        // Open the appointments file for reading
        ifstream file("appointments.txt", ios::in);
        if (!file) {
            cerr << "Error opening appointments file.\n";
            return;
        }

        string line;
        // Iterate through all records in the file
        while (getline(file, line)) {
            // Skip records marked as deleted
            if (line[0] != '*') {
                istringstream recordStream(line);
                string status, length, appointmentID, date, doctorID;

                // Parse the record fields
                getline(recordStream, status, '|');       // Read status field
                getline(recordStream, length, '|');      // Read length field
                getline(recordStream, appointmentID, '|'); // Read appointment ID
                getline(recordStream, date, '|');        // Read date field
                getline(recordStream, doctorID, '|');    // Read doctor ID

                // Remove padding characters from the date
                date.erase(remove(date.begin(), date.end(), '-'), date.end());

                // Output appointment details based on the user's choice
                switch (choice) {
                    case 0:  // Print all appointment details
                        cout << "ID: " << stoi(appointmentID)
                             << " | Date: " << date
                             << " | Doctor ID: " << stoi(doctorID) << '\n';
                        break;
                    case 1:  // Print only the Appointment ID
                        cout << "ID: " << stoi(appointmentID) << '\n';
                        break;
                    case 2:  // Print only the Date
                        cout << "Date: " << date << '\n';
                        break;
                    case 3:  // Print only the Doctor ID
                        cout << "Doctor ID: " << stoi(doctorID) << '\n';
                        break;
                    default:  // Default to printing all details
                        cout << "ID: " << stoi(appointmentID)
                             << " | Date: " << date
                             << " | Doctor ID: " << stoi(doctorID) << '\n';
                }
            }
        }

        // Close the appointments file
        file.close();
    }

};

#endif // HEALTHCAREMANAGEMENTSYSTEM_APPOINTMENTMANAGEMENTSYSTEM_H
