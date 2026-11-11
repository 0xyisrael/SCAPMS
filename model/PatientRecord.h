#pragma once
#include <string>
#include <vector>
#include "Appointable.h"

namespace clinic {

class PatientRecord : public Appointable {
public:
    PatientRecord(int id, const std::string& name, int age, const std::string& contact);
    virtual ~PatientRecord() = default;

    virtual std::string getCategory() const = 0;
    virtual int getPriority() const = 0;
    virtual std::string describe() const = 0;

    bool scheduleAppointment(const Appointment& appt) override;
    bool cancelAppointment(int appointmentId) override;

    int getId() const { return id; }
    std::string getName() const { return name; }
    int getAge() const { return age; }
    std::string getContact() const { return contact; }

    void setName(const std::string& n);
    void setAge(int a);
    void setContact(const std::string& c);

    const std::vector<Appointment>& getAppointments() const { return appointments; }
    std::vector<Appointment>& getAppointments() { return appointments; }
    int completedCount() const;
    int pendingCount() const;

protected:
    int id;
    std::string name;
    int age;
    std::string contact;
    std::vector<Appointment> appointments;
};
}
