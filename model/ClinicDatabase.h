#pragma once
#include <vector>
#include "PatientRecord.h"
#include "ClinicException.h"

namespace clinic {

class ClinicDatabase {
public:
    ClinicDatabase() = default;
    ~ClinicDatabase() { for (auto* p : patients) delete p; }
    ClinicDatabase(const ClinicDatabase&) = delete;
    ClinicDatabase& operator=(const ClinicDatabase&) = delete;

    void addPatient(PatientRecord* p) {
        if (!p) throw ClinicException("Invalid patient.");
        if (indexOf(p->getId()) != -1) {
            delete p;
            throw ClinicException("Patient ID already exists.");
        }
        patients.push_back(p);
    }

    // Removes the patient from the list but does NOT delete it.
    // The caller takes ownership (needed later for Undo).
    PatientRecord* removePatient(int id) {
        int i = indexOf(id);
        if (i == -1) throw ClinicException("Patient not found.");
        PatientRecord* p = patients[i];
        patients.erase(patients.begin() + i);
        return p;
    }

    std::vector<PatientRecord*>& getAll() { return patients; }
    const std::vector<PatientRecord*>& getAll() const { return patients; }
    size_t size() const { return patients.size(); }

private:
    std::vector<PatientRecord*> patients;   // required: std::vector

    int indexOf(int id) const {
        for (size_t i = 0; i < patients.size(); i++)
            if (patients[i]->getId() == id) return static_cast<int>(i);
        return -1;
    }
};
}
