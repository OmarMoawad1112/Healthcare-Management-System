#ifndef QUERYHANDLER_H
#define QUERYHANDLER_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include "DoctorManagementSystem.h"
#include "AppointmentManagementSystem.h"
#include "PrimaryIndex.h"

using namespace std;

class QueryHandler {
public:
    // Constructor initializes the Doctor and Appointment Management Systems
    QueryHandler(DoctorManagementSystem &doctorSys, AppointmentManagementSystem &appointmentSys)
            : doctorSystem(doctorSys), appointmentSystem(appointmentSys) {}

    // Handles user queries by parsing and executing SQL-like queries
    void handleUserQuery() {
        cout << "Enter your query: ";
        cin.ignore();
        string query;
        getline(cin, query);

        // Trim leading and trailing spaces from the query
        trim(query);

        // Convert the query to lowercase for case-insensitive comparison
        for(int i = 0; i < query.size(); ++i) {
            if (query[i] >= 'A' && query[i] <= 'Z') {
                query[i] = tolower(query[i]);
            }
        }

        // Remove the trailing semicolon if it exists
        if (!query.empty() && query.back() == ';') {
            query.pop_back();
        }

        // Validate the query format (must start with 'select' and contain 'from')
        if (query.substr(0, 6) != "select" || query.find("from") == string::npos) {
            cout << "Invalid query format. Please use: SELECT <fields> FROM <table> WHERE <condition>;\n";
            return;
        }

        // Extract parts of the query (fields, table, condition)
        size_t selectPos = query.find("select");
        size_t fromPos = query.find("from");
        size_t wherePos = query.find("where");

        string fields = query.substr(selectPos + 6, fromPos - (selectPos + 6));
        string table = query.substr(fromPos + 4, (wherePos == string::npos ? query.size() : wherePos) - (fromPos + 4));
        string condition = (wherePos != string::npos) ? query.substr(wherePos + 5) : "";

        // Trim spaces around extracted fields, table, and condition
        trim(fields);
        trim(table);
        trim(condition);

        // Check if the condition is 'id' and apply padding for doctors or appointments
        if ((table == "doctors" || table == "appointments") && !condition.empty()) {
            string key, value;
            if (parseCondition(condition, key, value)) {
                if (key == "id") {
                    // Apply 2-byte padding for the ID value
                    if (value.size() == 1) {
                        value = "0" + value;  // Pad with leading '0' if the value is a single digit
                    }
                    // Update the condition with the padded value
                    condition = key + " = '" + value + "'";
                }
            }
        }

        // Process the query based on table and condition
        if (table == "doctors") {
            if (isFileEmpty("DoctorPrimaryIndex.txt")) {
                cout << "doctors file is empty, insert records first.\n";
            }
            handleDoctorQuery(fields, condition);
        } else if (table == "appointments") {
            if (isFileEmpty("AppointmentPrimaryIndex.txt")) {
                cout << "appointments file is empty, insert records first.\n";
            }
            handleAppointmentQuery(fields, condition);
        } else {
            cout << "Invalid table name. Only 'doctors' and 'appointments' are supported.\n";
        }
    }

private:
    DoctorManagementSystem &doctorSystem;
    AppointmentManagementSystem &appointmentSystem;

    // Checks if a file is empty by opening it in binary mode and checking its size
    bool isFileEmpty(const string& fileName) {
        ifstream file(fileName, std::ios::ate | std::ios::binary); // Open in binary mode at the end
        return file.tellg() == 0; // If tellg() returns 0, the file is empty
    }

    // Trims leading and trailing spaces from a string
    void trim(string &str) {
        if (str.empty()) {
            return; // Handle empty string case
        }

        int i = 0, j = str.size() - 1;

        // Move i forward until the first non-space character
        while (i <= j && str[i] == ' ') {
            i++;
        }

        // Move j backward until the last non-space character
        while (j >= i && str[j] == ' ') {
            j--;
        }

        // Update the string only if trimming is required
        if (i != 0 || (j != str.size() - 1)) {
            str = str.substr(i, j - i + 1);
        }
    }

    // Parses a WHERE condition into key and value parts
    bool parseCondition(const string &condition, string &key, string &value) {
        size_t eqPos = condition.find('=');
        if (eqPos == string::npos) return false;

        key = condition.substr(0, eqPos);
        value = condition.substr(eqPos + 1);
        trim(key);
        trim(value);

        if (!value.empty() && value.front() == '\'' && value.back() == '\'') {
            value = value.substr(1, value.size() - 2);
        }
        return true;
    }

    // Handles doctor-related queries with or without conditions
    void handleDoctorQuery(const string &fields, const string &condition) {
        if (condition.empty()) {
            handleDoctorNoCondition(fields);
            return;
        }

        string key, value;
        if (!parseCondition(condition, key, value)) {
            cout << "Invalid WHERE condition. Use the format: <key>=<value>.\n";
            return;
        }

        if (key == "id") {
            handleDoctorById(fields, value);
        } else if (key == "name") {
            handleDoctorByName(fields, value);
        } else if (key == "address") {
            handleDoctorByAddress(fields, value);
        } else {
            cout << "Invalid WHERE condition. Valid keys for Doctor are 'ID' or 'Name'.\n";
        }
    }

    // Handles doctor queries with no conditions
    void handleDoctorNoCondition(const string &fields) {
        if (fields == "*" || fields == "all") {
            doctorSystem.printAllDoctors(0);
        } else if (fields == "id") {
            doctorSystem.printAllDoctors(1);
        } else if (fields == "name") {
            doctorSystem.printAllDoctors(2);
        } else if (fields == "address") {
            doctorSystem.printAllDoctors(3);
        } else {
            cout << "Invalid field in SELECT query for Doctor.\n";
        }
    }

    // Handles doctor queries filtered by ID
    void handleDoctorById(const string &fields, const string &id) {
        if (fields == "*" || fields == "all") {
            doctorSystem.printDoctorById(id, 0);  // Assuming this prints all doctor info
        } else if (fields == "id") {
            doctorSystem.printDoctorById(id, 1);  // Print only ID
        } else if (fields == "name") {
            doctorSystem.printDoctorById(id, 2);  // Print only Name
        } else if (fields == "address") {
            doctorSystem.printDoctorById(id, 3);  // Print only Address
        } else {
            cout << "Invalid field for Doctor: " << fields << ".\n";
        }
    }

    // Handles doctor queries filtered by Name
    void handleDoctorByName(const string &fields, const string &name) {
        vector<string> doctorIds = doctorSystem.searchDoctorsByName(name);
        if (doctorIds.empty()) {
            cout << "No doctors found with name: " << name << ".\n";
            return;
        }

        for (const string &doctorId: doctorIds) {
            if (fields == "*" || fields == "all") {
                doctorSystem.printDoctorById(doctorId, 0);
            } else if (fields == "id") {
                doctorSystem.printDoctorById(doctorId, 1);
            } else if (fields == "name") {
                doctorSystem.printDoctorById(doctorId, 2);
            } else if (fields == "address") {
                doctorSystem.printDoctorById(doctorId, 3);
            } else {
                cout << "Invalid field for Doctor: " << fields << ".\n";
            }
        }
    }

    // Handles doctor queries filtered by Address
    void handleDoctorByAddress(const string &fields, const string &address) {
        if (fields == "*" || fields == "all") {
            doctorSystem.printDoctorByAddress(address, 0);
        } else if (fields == "id") {
            doctorSystem.printDoctorByAddress(address, 1);
        } else if (fields == "name") {
            doctorSystem.printDoctorByAddress(address, 2);
        } else if (fields == "address") {
            doctorSystem.printDoctorByAddress(address, 3);
        } else {
            cout << "Invalid field for Doctor: " << fields << ".\n";
        }
    }

    // Handles appointment-related queries with or without conditions
    void handleAppointmentQuery(const string &fields, const string &condition) {
        if (condition.empty()) {
            handleAppointmentNoCondition(fields);
            return;
        }

        string key, value;
        if (!parseCondition(condition, key, value)) {
            cout << "Invalid WHERE condition for Appointment.\n";
            return;
        }

        if (key == "id") {
            handleAppointmentById(fields, value);
        } else if (key == "doctorid") {
            handleAppointmentByDoctorId(fields, value);
        } else if (key == "date") {
            handleAppointmentByDate(fields, value);
        } else {
            cout << "Invalid WHERE condition. Valid keys for Appointment are 'id'.\n";
        }
    }

    // Handles appointment queries with no conditions
    void handleAppointmentNoCondition(const string &fields) {
        if (fields == "*" || fields == "all") {
            appointmentSystem.printAllAppointments(0);
        } else if (fields == "id") {
            appointmentSystem.printAllAppointments(1);
        } else if (fields == "date") {
            appointmentSystem.printAllAppointments(2);
        } else if (fields == "doctor_id") {
            appointmentSystem.printAllAppointments(3);
        } else {
            cout << "Invalid field in SELECT query for Appointment.\n";
        }
    }

    // Handles appointment queries filtered by ID
    void handleAppointmentById(const string &fields, const string &id) {
        PrimaryIndex appointmentPrimaryIndex = appointmentSystem.getAppointmentPrimaryIndex();
        int offset = appointmentPrimaryIndex.binarySearchPrimaryIndex(id);
        if (offset == -1) {
            cout << "Appointment with ID " << id << " not found.\n";
            return;
        }

        if (fields == "*" || fields == "all") {
            appointmentSystem.printAppointmentById(id, 0);
        } else if (fields == "id") {
            appointmentSystem.printAppointmentById(id, 1);
        } else if (fields == "date") {
            appointmentSystem.printAppointmentById(id, 2);
        } else if (fields == "doctor id") {
            appointmentSystem.printAppointmentById(id, 3);
        } else {
            cout << "Invalid field for Appointment: " << fields << ".\n";
        }
    }

    // Handles appointment queries filtered by Doctor ID
    void handleAppointmentByDoctorId(const string &fields, const string &doctorId) {
        vector<string> appointmentIds = appointmentSystem.searchAppointmentsByDoctorID(doctorId);
        if (appointmentIds.empty()) {
            cout << "No appointments found for Doctor ID: " << doctorId << ".\n";
            return;
        }

        for (const string &appointmentId: appointmentIds) {
            if (fields == "*" || fields == "all") {
                appointmentSystem.printAppointmentById(appointmentId, 0);
            } else if (fields == "id") {
                appointmentSystem.printAppointmentById(appointmentId, 1);
            } else if (fields == "date") {
                appointmentSystem.printAppointmentById(appointmentId, 2);
            } else if (fields == "doctor_id") {
                appointmentSystem.printAppointmentById(appointmentId, 3);
            } else {
                cout << "Invalid field for Appointment: " << fields << ".\n";
            }
        }
    }

    // Handles appointment queries filtered by Date
    void handleAppointmentByDate(const string &fields, const string &date) {
        if (fields == "*" || fields == "all") {
            appointmentSystem.printAppointmentByDate(date, 0);
        } else if (fields == "id") {
            appointmentSystem.printAppointmentByDate(date, 1);
        } else if (fields == "date") {
            appointmentSystem.printAppointmentByDate(date, 2);
        } else if (fields == "doctor_id") {
            appointmentSystem.printAppointmentByDate(date, 3);
        } else {
            cout << "Invalid field for Appointment: " << fields << ".\n";
        }
    }
};

#endif // QUERYHANDLER_H
