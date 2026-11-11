#include "Appointment.h"
#include "ClinicException.h"

namespace clinic {

Appointment::Appointment(int id, int patientId, const std::string& date,
                         const std::string& time, const std::string& reason,
                         AppointmentStatus status)
    : id(id), patientId(patientId), date(date), time(time),
      reason(reason), status(status)
{
    if (date.size() != 10 || date[4] != '-' || date[7] != '-')
        throw ClinicException("Invalid appointment date. Use YYYY-MM-DD.");
    if (time.size() != 5 || time[2] != ':')
        throw ClinicException("Invalid appointment time. Use HH:MM.");
}

std::string Appointment::statusText() const {
    switch (status) {
        case AppointmentStatus::Pending:   return "Pending";
        case AppointmentStatus::Completed: return "Completed";
        default:                           return "Cancelled";
    }
}
}
