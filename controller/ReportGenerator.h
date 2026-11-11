#pragma once
#include <sstream>
#include <string>
#include <vector>
#include "ClinicManager.h"

namespace clinic {
namespace ReportGenerator {

// Report 1: patients with the most completed appointments
inline std::string mostFrequentPatients(const ClinicManager& m, size_t topN = 5) {
    std::vector<const PatientRecord*> list(m.database().getAll().begin(),
                                           m.database().getAll().end());
    // Selection sort, highest completed count first (written by hand)
    for (size_t i = 0; i < list.size(); i++) {
        size_t best = i;
        for (size_t j = i + 1; j < list.size(); j++)
            if (list[j]->completedCount() > list[best]->completedCount()) best = j;
        std::swap(list[i], list[best]);
    }
    std::ostringstream out;
    out << "MOST FREQUENTLY ATTENDED PATIENTS\n";
    bool any = false;
    for (size_t i = 0; i < list.size() && i < topN; i++) {
        if (list[i]->completedCount() == 0) break;
        out << "  " << list[i]->getName() << " (ID " << list[i]->getId() << "): "
            << list[i]->completedCount() << " completed\n";
        any = true;
    }
    if (!any) out << "  No completed appointments yet.\n";
    return out.str();
}

// Report 2: patients who still have pending appointments
inline std::string pendingAppointments(const ClinicManager& m) {
    std::ostringstream out;
    out << "PATIENTS WITH PENDING APPOINTMENTS\n";
    bool any = false;
    for (const PatientRecord* p : m.database().getAll()) {
        for (const Appointment& a : p->getAppointments()) {
            if (a.getStatus() == AppointmentStatus::Pending) {
                out << "  " << p->getName() << " (ID " << p->getId() << ") - "
                    << a.getDate() << " " << a.getTime() << " - " << a.getReason() << "\n";
                any = true;
            }
        }
    }
    if (!any) out << "  None.\n";
    return out.str();
}

// Report 3: how many patients in each category
inline std::string categoryDistribution(const ClinicManager& m) {
    int general = 0, emergency = 0, followUp = 0;
    for (const PatientRecord* p : m.database().getAll()) {
        std::string c = p->getCategory();
        if (c == "General") general++;
        else if (c == "Emergency") emergency++;
        else followUp++;
    }
    std::ostringstream out;
    out << "PATIENT CATEGORY DISTRIBUTION\n"
        << "  General Patients:   " << general << "\n"
        << "  Emergency Patients: " << emergency << "\n"
        << "  Follow-Up Patients: " << followUp << "\n";
    return out.str();
}

// Report 4: appointment totals for one date (YYYY-MM-DD). Empty date = all dates.
inline std::string dailySummary(const ClinicManager& m, const std::string& date = "") {
    int total = 0, completed = 0, pending = 0, cancelled = 0;
    for (const PatientRecord* p : m.database().getAll()) {
        for (const Appointment& a : p->getAppointments()) {
            if (!date.empty() && a.getDate() != date) continue;
            total++;
            if (a.getStatus() == AppointmentStatus::Completed) completed++;
            else if (a.getStatus() == AppointmentStatus::Pending) pending++;
            else cancelled++;
        }
    }
    std::ostringstream out;
    out << "APPOINTMENT SUMMARY (" << (date.empty() ? "all dates" : date) << ")\n"
        << "  Total Appointments: " << total << "\n"
        << "  Completed: " << completed << "\n"
        << "  Pending:   " << pending << "\n"
        << "  Cancelled: " << cancelled << "\n";
    return out.str();
}
}
}
