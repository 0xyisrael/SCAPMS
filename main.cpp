#include "gui/MainWindow.h"
using namespace clinic;

int main() {
    ClinicManager manager;
    try {
        manager.addPatient(new GeneralPatient(105, "Ngozi Okafor", 34, "08031110005"));
        manager.addPatient(new EmergencyPatient(101, "Tunde Bello", 40, "08031110001", 4));
        manager.addPatient(new GeneralPatient(108, "Zainab Yusuf", 67, "08031110008"));
        manager.addPatient(new FollowUpPatient(103, "Chioma Eze", 30, "08031110003", "Malaria"));
        manager.addPatient(new GeneralPatient(107, "Emeka Nwosu", 19, "08031110007"));
        manager.addPatient(new EmergencyPatient(102, "Ibrahim Musa", 52, "08031110002", 5));
        manager.addPatient(new FollowUpPatient(106, "Funke Adebayo", 45, "08031110006", "Hypertension"));
        manager.addPatient(new GeneralPatient(104, "Ada Obi", 25, "08031110004"));

        int a1 = manager.scheduleAppointment(105, "2026-11-12", "09:00", "Checkup");
        int a2 = manager.scheduleAppointment(101, "2026-11-12", "10:00", "Injury");
        manager.scheduleAppointment(103, "2026-11-13", "11:00", "Review");
        manager.scheduleAppointment(105, "2026-11-13", "14:00", "Lab results");
        manager.scheduleAppointment(106, "2026-11-14", "09:30", "Blood pressure check");
        manager.completeAppointment(105, a1);
        manager.completeAppointment(101, a2);

        manager.addToQueue(101);
        manager.addToQueue(104);
    } catch (const ClinicException&) {}

    gui::MainWindow win(manager);
    win.show();
    return Fl::run();
}
