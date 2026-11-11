#pragma once
#include <queue>
#include <stack>
#include <string>
#include <vector>
#include "../model/ClinicDatabase.h"
#include "../model/GeneralPatient.h"
#include "../model/EmergencyPatient.h"
#include "../model/FollowUpPatient.h"
#include "SearchEngine.h"

namespace clinic {

enum class UndoType { DeletePatient, ModifyPatient, CancelAppointment };

// One entry on the undo stack: remembers what was done and how to reverse it
struct UndoAction {
    UndoType type = UndoType::DeletePatient;
    PatientRecord* deleted = nullptr;       // used when a patient was deleted
    int patientId = 0;
    std::string oldName, oldContact;        // used when a patient was edited
    int oldAge = 0;
    int appointmentId = 0;                  // used when an appointment was cancelled
    AppointmentStatus oldStatus = AppointmentStatus::Pending;
};

class ClinicManager {
public:
    static constexpr int CACHE_SIZE = 10;

    ClinicManager() { for (int i = 0; i < CACHE_SIZE; i++) recent[i] = nullptr; }
    ~ClinicManager() { clearUndoStack(); }
    ClinicManager(const ClinicManager&) = delete;
    ClinicManager& operator=(const ClinicManager&) = delete;

    ClinicDatabase& database() { return db; }
    const ClinicDatabase& database() const { return db; }

    // ---------- Patients ----------
    void addPatient(PatientRecord* p) { db.addPatient(p); }

    // Looks in the array cache first, then does a linear search
    PatientRecord* findPatient(int id) {
        for (int i = 0; i < CACHE_SIZE; i++)
            if (recent[i] && recent[i]->getId() == id) return recent[i];
        int idx = SearchEngine::linearSearchById(db.getAll(), id);
        if (idx == -1) return nullptr;
        addToCache(db.getAll()[idx]);
        return db.getAll()[idx];
    }

    void deletePatient(int id) {
        PatientRecord* p = db.removePatient(id);      // throws if not found
        removeFromCache(p);
        UndoAction a;
        a.type = UndoType::DeletePatient;
        a.deleted = p;
        a.patientId = id;
        undoStack.push(a);
    }

    void editPatient(int id, const std::string& name, int age, const std::string& contact) {
        PatientRecord* p = findPatient(id);
        if (!p) throw ClinicException("Patient not found.");
        UndoAction a;
        a.type = UndoType::ModifyPatient;
        a.patientId = id;
        a.oldName = p->getName();
        a.oldAge = p->getAge();
        a.oldContact = p->getContact();
        try {
            p->setName(name);
            p->setAge(age);
            p->setContact(contact);
        } catch (...) {
            p->setName(a.oldName);       // put the old values back, then report the error
            p->setAge(a.oldAge);
            p->setContact(a.oldContact);
            throw;
        }
        undoStack.push(a);
    }

    // ---------- Appointments ----------
    int scheduleAppointment(int patientId, const std::string& date,
                            const std::string& time, const std::string& reason) {
        PatientRecord* p = findPatient(patientId);
        if (!p) throw ClinicException("Patient not found.");
        int id = nextAppointmentId;
        Appointment appt(id, patientId, date, time, reason);   // throws if date/time invalid
        p->scheduleAppointment(appt);
        nextAppointmentId++;
        return id;
    }

    void cancelAppointment(int patientId, int appointmentId) {
        PatientRecord* p = findPatient(patientId);
        if (!p) throw ClinicException("Patient not found.");
        Appointment* ap = findAppointment(p, appointmentId);
        if (!ap) throw ClinicException("Appointment not found.");
        if (ap->getStatus() != AppointmentStatus::Pending)
            throw ClinicException("Only pending appointments can be cancelled.");
        UndoAction a;
        a.type = UndoType::CancelAppointment;
        a.patientId = patientId;
        a.appointmentId = appointmentId;
        a.oldStatus = ap->getStatus();
        p->cancelAppointment(appointmentId);
        undoStack.push(a);
    }

    void completeAppointment(int patientId, int appointmentId) {
        PatientRecord* p = findPatient(patientId);
        if (!p) throw ClinicException("Patient not found.");
        Appointment* ap = findAppointment(p, appointmentId);
        if (!ap) throw ClinicException("Appointment not found.");
        if (ap->getStatus() != AppointmentStatus::Pending)
            throw ClinicException("Only pending appointments can be completed.");
        ap->setStatus(AppointmentStatus::Completed);
    }

    int getNextAppointmentId() const { return nextAppointmentId; }
    void setNextAppointmentId(int n) { nextAppointmentId = n; }

    // ---------- Waiting queue (std::queue, first in = first out) ----------
    void addToQueue(int patientId) {
        if (!findPatient(patientId)) throw ClinicException("Patient not found.");
        for (int id : waitingIds())
            if (id == patientId) throw ClinicException("Patient is already in the queue.");
        waiting.push(patientId);
    }

    // Returns the next patient to see, or nullptr if nobody is waiting
    PatientRecord* callNext() {
        while (!waiting.empty()) {
            int id = waiting.front();
            waiting.pop();
            PatientRecord* p = findPatient(id);
            if (p) return p;              // skip anyone who was deleted meanwhile
        }
        return nullptr;
    }

    size_t waitingCount() const { return waiting.size(); }

    std::vector<int> waitingIds() const {
        std::vector<int> ids;
        std::queue<int> copy = waiting;   // copy so the real queue is untouched
        while (!copy.empty()) { ids.push_back(copy.front()); copy.pop(); }
        return ids;
    }

    // ---------- Undo (std::stack) ----------
    bool canUndo() const { return !undoStack.empty(); }

    std::string undoLast() {
        if (undoStack.empty()) throw ClinicException("Nothing to undo.");
        UndoAction a = undoStack.top();
        undoStack.pop();
        switch (a.type) {
            case UndoType::DeletePatient: {
                std::string name = a.deleted->getName();
                db.addPatient(a.deleted);
                return "Restored deleted patient " + name;
            }
            case UndoType::ModifyPatient: {
                PatientRecord* p = findPatient(a.patientId);
                if (!p) throw ClinicException("Patient no longer exists.");
                p->setName(a.oldName);
                p->setAge(a.oldAge);
                p->setContact(a.oldContact);
                return "Reverted edit of patient " + std::to_string(a.patientId);
            }
            case UndoType::CancelAppointment: {
                PatientRecord* p = findPatient(a.patientId);
                if (!p) throw ClinicException("Patient no longer exists.");
                Appointment* ap = findAppointment(p, a.appointmentId);
                if (!ap) throw ClinicException("Appointment no longer exists.");
                ap->setStatus(a.oldStatus);
                return "Restored appointment " + std::to_string(a.appointmentId);
            }
        }
        return "Undone";
    }

    // Wipes everything (used before loading a file)
    void resetAll() {
        for (int i = 0; i < CACHE_SIZE; i++) recent[i] = nullptr;
        nextSlot = 0;
        std::queue<int> empty;
        std::swap(waiting, empty);
        clearUndoStack();
        while (db.size() > 0)
            delete db.removePatient(db.getAll().front()->getId());
        nextAppointmentId = 1;
    }

private:
    ClinicDatabase db;
    std::queue<int> waiting;                // patient IDs waiting for consultation
    std::stack<UndoAction> undoStack;       // administrative operations
    PatientRecord* recent[CACHE_SIZE];      // fixed-size array cache
    int nextSlot = 0;
    int nextAppointmentId = 1;

    void addToCache(PatientRecord* p) {
        for (int i = 0; i < CACHE_SIZE; i++)
            if (recent[i] == p) return;
        recent[nextSlot] = p;
        nextSlot = (nextSlot + 1) % CACHE_SIZE;   // oldest entry gets replaced
    }

    void removeFromCache(PatientRecord* p) {
        for (int i = 0; i < CACHE_SIZE; i++)
            if (recent[i] == p) recent[i] = nullptr;
    }

    Appointment* findAppointment(PatientRecord* p, int id) {
        for (auto& a : p->getAppointments())
            if (a.getId() == id) return &a;
        return nullptr;
    }

    void clearUndoStack() {
        while (!undoStack.empty()) {
            delete undoStack.top().deleted;   // deleting nullptr is safe
            undoStack.pop();
        }
    }
};
}
