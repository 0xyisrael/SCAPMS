#include "gui/MainWindow.h"
using namespace clinic;

int main() {
    ClinicManager manager;
    try {
        manager.addPatient(new GeneralPatient(1, "Ada Obi", 25, "0803111111"));
        manager.addPatient(new EmergencyPatient(2, "Tunde Bello", 40, "0805222222", 4));
        manager.addPatient(new FollowUpPatient(3, "Chioma Eze", 30, "0807333333", "Malaria"));
        manager.scheduleAppointment(1, "2026-11-02", "09:00", "Checkup");
        manager.addToQueue(2);
    } catch (const ClinicException&) {}

    gui::MainWindow win(manager);
    win.show();
    return Fl::run();
}
