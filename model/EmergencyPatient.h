#pragma once
#include "PatientRecord.h"
#include "ClinicException.h"

namespace clinic {
class EmergencyPatient : public PatientRecord {
public:
    EmergencyPatient(int id, const std::string& name, int age,
                     const std::string& contact, int severity)
        : PatientRecord(id, name, age, contact) { setSeverity(severity); }

    void setSeverity(int s) {
        if (s < 1 || s > 5) throw ClinicException("Severity must be between 1 and 5.");
        severity = s;
    }
    int getSeverity() const { return severity; }

    std::string getCategory() const override { return "Emergency"; }
    int getPriority() const override { return 1; }
    std::string describe() const override {
        return "EMERGENCY patient " + name + " - severity " +
               std::to_string(severity) + "/5. Attend immediately.";
    }
private:
    int severity;
};
}
