#pragma once
#include <fstream>
#include <string>
#include <vector>
#include "../controller/ClinicManager.h"

namespace clinic {
namespace FileHandler {

// Commas would break the CSV format, so replace them with spaces
inline std::string clean(std::string s) {
    for (char& c : s) if (c == ',' || c == '\n' || c == '\r') c = ' ';
    return s;
}

inline std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> parts;
    std::string cur;
    for (char c : line) {
        if (c == ',') { parts.push_back(cur); cur.clear(); }
        else if (c != '\r') cur += c;
    }
    parts.push_back(cur);
    return parts;
}

inline void save(const ClinicManager& m, const std::string& path) {
    std::ofstream out(path);
    if (!out) throw ClinicException("Could not open file for saving: " + path);

    for (const PatientRecord* p : m.database().getAll()) {
        std::string extra;
        if (auto e = dynamic_cast<const EmergencyPatient*>(p))
            extra = std::to_string(e->getSeverity());
        else if (auto f = dynamic_cast<const FollowUpPatient*>(p))
            extra = clean(f->getPreviousDiagnosis());

        out << "P," << p->getCategory() << "," << p->getId() << ","
            << clean(p->getName()) << "," << p->getAge() << ","
            << clean(p->getContact()) << "," << extra << "\n";

        for (const Appointment& a : p->getAppointments())
            out << "A," << a.getPatientId() << "," << a.getId() << ","
                << a.getDate() << "," << a.getTime() << ","
                << clean(a.getReason()) << "," << a.statusText() << "\n";
    }
    for (int id : m.waitingIds()) out << "Q," << id << "\n";
    if (!out) throw ClinicException("Error while writing the file.");
}

// Reads the whole file into a temporary manager first. Only if the file is
// completely valid does it replace the current data, so a bad file never
// destroys what is already loaded.
inline void load(ClinicManager& m, const std::string& path) {
    std::ifstream in(path);
    if (!in) throw ClinicException("File not found or cannot be opened: " + path);

    ClinicManager temp;
    std::vector<int> queueIds;
    int maxAppt = 0, lineNo = 0;
    std::string line;

    try {
        while (std::getline(in, line)) {
            lineNo++;
            if (line.empty()) continue;
            std::vector<std::string> f = split(line);

            if (f[0] == "P") {
                if (f.size() != 7) throw ClinicException("Wrong number of fields.");
                int id = std::stoi(f[2]);
                int age = std::stoi(f[4]);
                PatientRecord* p = nullptr;
                if (f[1] == "General")        p = new GeneralPatient(id, f[3], age, f[5]);
                else if (f[1] == "Emergency") p = new EmergencyPatient(id, f[3], age, f[5], std::stoi(f[6]));
                else if (f[1] == "Follow-Up") p = new FollowUpPatient(id, f[3], age, f[5], f[6]);
                else throw ClinicException("Unknown patient category.");
                temp.addPatient(p);
            }
            else if (f[0] == "A") {
                if (f.size() != 7) throw ClinicException("Wrong number of fields.");
                int pid = std::stoi(f[1]);
                int aid = std::stoi(f[2]);
                AppointmentStatus st;
                if (f[6] == "Pending")        st = AppointmentStatus::Pending;
                else if (f[6] == "Completed") st = AppointmentStatus::Completed;
                else if (f[6] == "Cancelled") st = AppointmentStatus::Cancelled;
                else throw ClinicException("Unknown appointment status.");
                PatientRecord* p = temp.findPatient(pid);
                if (!p) throw ClinicException("Appointment refers to a missing patient.");
                p->scheduleAppointment(Appointment(aid, pid, f[3], f[4], f[5], st));
                if (aid > maxAppt) maxAppt = aid;
            }
            else if (f[0] == "Q") {
                if (f.size() != 2) throw ClinicException("Wrong number of fields.");
                queueIds.push_back(std::stoi(f[1]));
            }
            else throw ClinicException("Unknown record type.");
        }
    } catch (const ClinicException& e) {
        throw ClinicException("File error on line " + std::to_string(lineNo) + ": " + e.what());
    } catch (const std::exception&) {
        throw ClinicException("File error on line " + std::to_string(lineNo) + ": invalid number.");
    }

    // File was valid: replace the current data
    m.resetAll();
    while (temp.database().size() > 0) {
        PatientRecord* p = temp.database().removePatient(temp.database().getAll().front()->getId());
        m.addPatient(p);
    }
    m.setNextAppointmentId(maxAppt + 1);
    for (int id : queueIds) {
        try { m.addToQueue(id); } catch (const ClinicException&) {}
    }
}
}
}
