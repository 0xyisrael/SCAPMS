#pragma once
#include "PatientRecord.h"

namespace clinic {
class GeneralPatient : public PatientRecord {
public:
    GeneralPatient(int id, const std::string& name, int age, const std::string& contact)
        : PatientRecord(id, name, age, contact) {}

    std::string getCategory() const override { return "General"; }
    int getPriority() const override { return 2; }
    std::string describe() const override {
        return "General patient " + name + " - routine consultation.";
    }
};
}
