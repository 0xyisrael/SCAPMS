#pragma once
#include "Appointment.h"

namespace clinic {
class Appointable {
public:
    virtual ~Appointable() = default;
    virtual bool scheduleAppointment(const Appointment& appt) = 0;
    virtual bool cancelAppointment(int appointmentId) = 0;
};
}
