#include <bits/stdc++.h>
#include "DoctorManagementSystem.h"
#include "AppointmentManagementSystem.h"
#include "QueryHandler.h"

using namespace std;

// Trims leading and trailing spaces from a string
void trim(string &str) {
    if (str.empty()) return;

    int i = 0, j = str.size() - 1;
    while (i <= j && str[i] == ' ') i++; // Find the first non-space character
    while (j >= i && str[j] == ' ') j--; // Find the last non-space character

    // Extract the trimmed portion of the string
    if (i != 0 || j != str.size() - 1) {
        str = str.substr(i, j - i + 1);
    }
}

// Converts a string to lowercase
void toLower(string &str) {
    // Iterate through each character and convert to lowercase
    for (int i = 0; i < str.size(); ++i) {
        if (str[i] >= 'A' && str[i] <= 'Z') {
            str[i] = tolower(str[i]);
        }
    }
}

// Ensures the user presses 'y' to continue
void checkContinue() {
    string cont = "0";
    while (cont != "y") {
        cout << "Press 'y' or 'Y' to continue: ";
        cin >> cont;     // Read user input
        toLower(cont);   // Convert input to lowercase
    }
}

// Pads an integer with leading zeros to make it two characters long
string padInt(int x) {
    string paddedInt = to_string(x);
    if (paddedInt.length() < 2) {
        paddedInt.insert(0, 1, '0'); // Add a leading zero if needed
    }
    return paddedInt;
}

int main() {
    cout << "Welcome to Your Health Care Management System\n";
    int choice = -1;

    // Initialize the doctor management system
    DoctorManagementSystem doctorSystem;

    // Initialize the appointment system, linking it with the doctor system
    AppointmentManagementSystem appointmentSystem(doctorSystem.getDoctorPrimaryIndex());

    // Initialize the query handler with both systems
    QueryHandler queryHandler(doctorSystem, appointmentSystem);

    // Main menu loop
    while (choice != 0) {
        // Display menu options
        cout <<
             "1) Add New Doctor\n"
             "2) Add New Appointment\n"
             "3) Update Doctor Name (Doctor ID)\n"
             "4) Update Appointment Date (Appointment ID)\n"
             "5) Delete Appointment (Appointment ID)\n"
             "6) Delete Doctor (Doctor ID)\n"
             "7) Print Doctor Info (Doctor ID)\n"
             "8) Print Appointment Info (Appointment ID)\n"
             "9) Write Query\n"
             "10) Print all doctors\n"
             "11) Print all appointments\n"
             "0) Exit\n"
             "Enter a choice: ";
        cin >> choice;

        if (choice == 1) {
            // Add a new doctor
            Doctor doctor;
            string name, address;

            cout << "Enter doctor name: ";
            cin.ignore();
            getline(cin, name); // Read the full name
            toLower(name);
            trim(name); // Trim spaces

            cout << "Enter doctor address: ";
            getline(cin, address);
            toLower(address);
            trim(address);

            doctor.name = name;
            doctor.address = address;

            doctorSystem.addDoctor(doctor); // Add the doctor to the system
            checkContinue();
        }
        else if (choice == 2) {
            // Add a new appointment
            Appointment appointment;
            string date;
            int doctorID;

            cout << "Enter the date: ";
            cin.ignore();
            getline(cin, date); // Read the date
            toLower(date);
            trim(date);

            cout << "Enter doctor ID: ";
            cin >> doctorID;
            string paddedDoctorId = padInt(doctorID); // Pad doctor ID

            appointment.date = date;
            appointment.doctorID = paddedDoctorId;

            appointmentSystem.addAppointment(appointment); // Add appointment to the system
            checkContinue();
        }
        else if (choice == 3) {
            // Update doctor name
            int id;
            cout << "Please enter the Doctor's ID you want to change his name: ";
            cin >> id;

            string paddedId = padInt(id);
            string newName;

            cout << "Please enter Doctor's new name: ";
            cin.ignore();
            getline(cin, newName);
            toLower(newName);
            trim(newName);

            doctorSystem.updateDoctorName(paddedId, newName); // Update doctor name
            checkContinue();
        }
        else if (choice == 4) {
            // Update appointment date
            string id;
            cout << "Please enter the Appointment's ID you want to change its date: ";
            cin >> id;

            string newDate;
            cout << "Please enter new date: ";
            cin.ignore();
            getline(cin, newDate);
            toLower(newDate);
            trim(newDate);

            appointmentSystem.updateAppointmentDate(id, newDate); // Update appointment date
            checkContinue();
        }
        else if (choice == 5) {
            // Delete an appointment
            int id;
            cout << "Please enter the Appointment's ID you want to delete: ";
            cin >> id;

            string paddedId = padInt(id);
            appointmentSystem.deleteAppointment(paddedId); // Delete appointment
            checkContinue();
        }
        else if (choice == 6) {
            // Delete a doctor
            int id;
            cout << "Please enter the Doctor's ID you want to delete: ";
            cin >> id;

            string paddedId = padInt(id);
            doctorSystem.deleteDoctor(paddedId); // Delete doctor
            checkContinue();
        }
        else if (choice == 7) {
            // Print doctor info by ID
            int id;
            cout << "Please enter the Doctor's ID you want to search for: ";
            cin >> id;

            string paddedId = padInt(id);
            doctorSystem.printDoctorById(paddedId, 4); // Print doctor info
            checkContinue();
        }
        else if (choice == 8) {
            // Print appointment info by ID
            int id;
            cout << "Please enter the Appointment's ID you want to search for: ";
            cin >> id;

            string paddedId = padInt(id);
            appointmentSystem.printAppointmentById(paddedId, 4); // Print appointment info
            checkContinue();
        }
        else if (choice == 9) {
            // Handle a query
            cout << "Query Example: SELECT * FROM Doctors WHERE ID = '1';\n";
            queryHandler.handleUserQuery(); // Process the query
            checkContinue();
        }
        else if (choice == 10) {
            // Print all doctors
            doctorSystem.printAllDoctors(0); // Print list of all doctors
            checkContinue();
        }
        else if (choice == 11) {
            // Print all appointments
            appointmentSystem.printAllAppointments(0); // Print list of all appointments
            checkContinue();
        }
        else {
            // Handle invalid choice
            cout << "Enter a valid choice\n";
        }
    }

    // End of program
    cout << "End of program\n";
    return 0;
}
