#pragma once
#include <string>

namespace clinic {

enum class AppointmentStatus { Pending, Completed, Cancelled };

class Appointment {
public:
    Appointment(int id, int patientId, const std::string& date,
                const std::string& time, const std::string& reason,
                AppointmentStatus status = AppointmentStatus::Pending);

    int getId() const { return id; }
    int getPatientId() const { return patientId; }
    std::string getDate() const { return date; }
    std::string getTime() const { return time; }
    std::string getReason() const { return reason; }
    AppointmentStatus getStatus() const { return status; }
    void setStatus(AppointmentStatus s) { status = s; }
    std::string statusText() const;

private:
    int id;
    int patientId;
    std::string date;
    std::string time;
    std::string reason;
    AppointmentStatus status;
};
}
