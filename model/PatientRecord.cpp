#include "PatientRecord.h"
#include "ClinicException.h"

namespace clinic {

PatientRecord::PatientRecord(int id, const std::string& name, int age,
                             const std::string& contact)
    : id(id)
{
    setName(name);
    setAge(age);
    setContact(contact);
}

void PatientRecord::setName(const std::string& n) {
    if (n.empty()) throw ClinicException("Patient name cannot be empty.");
    name = n;
}

void PatientRecord::setAge(int a) {
    if (a < 0 || a > 120)
        throw ClinicException("Patient age must be between 0 and 120.");
    age = a;
}

void PatientRecord::setContact(const std::string& c) {
    if (c.empty()) throw ClinicException("Contact information cannot be empty.");
    contact = c;
}

bool PatientRecord::scheduleAppointment(const Appointment& appt) {
    for (const auto& a : appointments)
        if (a.getId() == appt.getId())
            throw ClinicException("Appointment ID already exists.");
    appointments.push_back(appt);
    return true;
}

bool PatientRecord::cancelAppointment(int appointmentId) {
    for (auto& a : appointments) {
        if (a.getId() == appointmentId) {
            a.setStatus(AppointmentStatus::Cancelled);
            return true;
        }
    }
    return false;
}

int PatientRecord::completedCount() const {
    int c = 0;
    for (const auto& a : appointments)
        if (a.getStatus() == AppointmentStatus::Completed) c++;
    return c;
}

int PatientRecord::pendingCount() const {
    int c = 0;
    for (const auto& a : appointments)
        if (a.getStatus() == AppointmentStatus::Pending) c++;
    return c;
}
}
