#pragma once
#include "PatientRecord.h"

namespace clinic {
class FollowUpPatient : public PatientRecord {
public:
    FollowUpPatient(int id, const std::string& name, int age,
                    const std::string& contact, const std::string& previousDiagnosis)
        : PatientRecord(id, name, age, contact), previousDiagnosis(previousDiagnosis) {}

    std::string getPreviousDiagnosis() const { return previousDiagnosis; }
    void setPreviousDiagnosis(const std::string& d) { previousDiagnosis = d; }

    std::string getCategory() const override { return "Follow-Up"; }
    int getPriority() const override { return 3; }
    std::string describe() const override {
        return "Follow-up patient " + name + " - previous diagnosis: " + previousDiagnosis;
    }
private:
    std::string previousDiagnosis;
};
}
