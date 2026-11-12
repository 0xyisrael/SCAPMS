#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <cctype>
#include <cstdint>
#include <ctime>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include "../controller/ClinicManager.h"

namespace clinic {
namespace gui {

class AppointmentPanel : public Fl_Group {
public:
    AppointmentPanel(int X, int Y, int W, int H, ClinicManager& mgr,
                     std::function<void(const std::string&)> notifyFn)
        : Fl_Group(X, Y, W, H, "Appointments"), m(mgr), notify(notifyFn)
    {
        box(FL_FLAT_BOX);

        // ---- Table of appointments ----
        browser = new Fl_Hold_Browser(X + 10, Y + 10, W - 400, H - 20);
        widths[0] = 45; widths[1] = 150; widths[2] = 85;
        widths[3] = 50; widths[4] = 80;  widths[5] = 0;
        browser->column_widths(widths);
        browser->callback(browserCb, this);
        browser->tooltip("Click an appointment to select it");

        // ---- Form (Fl_Pack stacks the widgets) ----
        form = new Fl_Pack(X + W - 280, Y + 10, 270, H - 20);
        form->type(Fl_Pack::VERTICAL);
        form->spacing(6);

        pidInput = new Fl_Int_Input(0, 0, 270, 26, "Patient ID:");
        pidInput->when(FL_WHEN_CHANGED);          // text field event: fires on every keystroke
        pidInput->callback(pidCb, this);
        pidInput->tooltip("ID of the patient the appointment is for");

        infoBox = new Fl_Box(0, 0, 270, 26, "");
        infoBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
        infoBox->labelcolor(FL_DARK_BLUE);

        dateInput = new Fl_Input(0, 0, 270, 26, "Date:");
        dateInput->value(today().c_str());
        dateInput->tooltip("Format: YYYY-MM-DD");
        timeInput = new Fl_Input(0, 0, 270, 26, "Time:");
        timeInput->value("09:00");
        timeInput->tooltip("24-hour format: HH:MM");
        reasonInput = new Fl_Input(0, 0, 270, 26, "Reason:");

        Fl_Button* schedBtn = new Fl_Button(0, 0, 270, 26, "Schedule Appointment");
        schedBtn->callback(schedCb, this);
        schedBtn->tooltip("Book an appointment with the details above");
        Fl_Button* cancelBtn = new Fl_Button(0, 0, 270, 26, "Cancel Selected");
        cancelBtn->callback(cancelCb, this);
        cancelBtn->tooltip("Cancel the selected pending appointment (can be undone)");
        Fl_Button* doneBtn = new Fl_Button(0, 0, 270, 26, "Mark Selected Completed");
        doneBtn->callback(doneCb, this);
        doneBtn->tooltip("Record that the selected appointment took place");
        Fl_Button* undoBtn = new Fl_Button(0, 0, 270, 26, "Undo Last Admin Operation");
        undoBtn->callback(undoCb, this);
        undoBtn->tooltip("Reverse the last delete, edit or appointment cancellation");
        form->end();

        end();
        resizable(browser);
        refresh();
    }

    void refresh() {
        int keepP = selP, keepA = selA;
        selP = selA = 0;
        browser->clear();
        rows.clear();
        browser->add((head("Appt") + "\t" + head("Patient") + "\t" + head("Date") + "\t" +
                      head("Time") + "\t" + head("Status") + "\t" + head("Reason")).c_str(),
                     (void*)(intptr_t)0);
        for (const PatientRecord* p : m.database().getAll()) {
            for (const Appointment& a : p->getAppointments()) {
                rows.push_back(std::make_pair(p->getId(), a.getId()));
                std::string c = colourFor(a.getStatus());
                std::string line = c + std::to_string(a.getId()) + "\t" + c + p->getName() + "\t" +
                                   c + a.getDate() + "\t" + c + a.getTime() + "\t" +
                                   c + a.statusText() + "\t" + c + a.getReason();
                browser->add(line.c_str(), (void*)(intptr_t)rows.size());
                if (p->getId() == keepP && a.getId() == keepA) {
                    browser->select((int)rows.size() + 1);
                    selP = keepP; selA = keepA;
                }
            }
        }
    }

private:
    ClinicManager& m;
    std::function<void(const std::string&)> notify;
    Fl_Hold_Browser* browser;
    Fl_Pack* form;
    Fl_Int_Input* pidInput;
    Fl_Box* infoBox;
    Fl_Input *dateInput, *timeInput, *reasonInput;
    int widths[6];
    std::vector<std::pair<int, int>> rows;   // (patientId, appointmentId) for each table row
    int selP = 0, selA = 0;

    // ---------- helpers ----------
    static std::string head(const std::string& s) { return "@b@." + s; }

    // Custom presentation: colour depends on appointment status
    static std::string colourFor(AppointmentStatus s) {
        if (s == AppointmentStatus::Cancelled) return "@C" + std::to_string((int)FL_RED) + "@.";
        if (s == AppointmentStatus::Completed) return "@C" + std::to_string((int)FL_DARK_GREEN) + "@.";
        return "@C" + std::to_string((int)FL_BLUE) + "@.";
    }

    static std::string today() {
        char b[16];
        std::time_t n = std::time(nullptr);
        std::strftime(b, sizeof b, "%Y-%m-%d", std::localtime(&n));
        return b;
    }

    static bool digits(const std::string& s) {
        if (s.empty()) return false;
        for (char c : s) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        return true;
    }

    static void validateDate(const std::string& d) {
        if (d.size() != 10 || d[4] != '-' || d[7] != '-' || !digits(d.substr(0, 4)) ||
            !digits(d.substr(5, 2)) || !digits(d.substr(8, 2)))
            throw ClinicException("Date must be in the format YYYY-MM-DD.");
        int mo = std::stoi(d.substr(5, 2)), da = std::stoi(d.substr(8, 2));
        if (mo < 1 || mo > 12 || da < 1 || da > 31)
            throw ClinicException("Invalid date: month must be 1-12 and day 1-31.");
    }

    static void validateTime(const std::string& t) {
        if (t.size() != 5 || t[2] != ':' || !digits(t.substr(0, 2)) || !digits(t.substr(3, 2)))
            throw ClinicException("Time must be in the format HH:MM.");
        int h = std::stoi(t.substr(0, 2)), mi = std::stoi(t.substr(3, 2));
        if (h > 23 || mi > 59)
            throw ClinicException("Invalid time: hours 00-23, minutes 00-59.");
    }

    static int readInt(Fl_Input* in, const std::string& what) {
        std::string s = in->value();
        if (s.empty()) throw ClinicException(what + " is required.");
        try { return std::stoi(s); }
        catch (...) { throw ClinicException(what + " must be a valid number."); }
    }

    void showInfo() {
        std::string s = pidInput->value();
        std::string text;
        if (!s.empty()) {
            try {
                PatientRecord* p = m.findPatient(std::stoi(s));
                text = p ? "Patient: " + p->getName() : "(no patient with that ID)";
            } catch (...) { text = "(invalid ID)"; }
        }
        infoBox->copy_label(text.c_str());
        infoBox->redraw();
    }

    // ---------- event handlers ----------
    static void pidCb(Fl_Widget*, void* d)    { static_cast<AppointmentPanel*>(d)->showInfo(); }
    static void browserCb(Fl_Widget*, void* d){ static_cast<AppointmentPanel*>(d)->onSelect(); }
    static void schedCb(Fl_Widget*, void* d)  { static_cast<AppointmentPanel*>(d)->onSchedule(); }
    static void cancelCb(Fl_Widget*, void* d) { static_cast<AppointmentPanel*>(d)->onCancel(); }
    static void doneCb(Fl_Widget*, void* d)   { static_cast<AppointmentPanel*>(d)->onComplete(); }
    static void undoCb(Fl_Widget*, void* d)   { static_cast<AppointmentPanel*>(d)->onUndo(); }

    void onSelect() {
        int line = browser->value();
        if (line <= 0) return;
        int idx = (int)(intptr_t)browser->data(line);
        if (idx == 0) { browser->deselect(); selP = selA = 0; return; }   // header row
        selP = rows[idx - 1].first;
        selA = rows[idx - 1].second;
        pidInput->value(std::to_string(selP).c_str());
        showInfo();
    }

    void onSchedule() {
        try {
            int pid = readInt(pidInput, "Patient ID");
            std::string date = dateInput->value();
            std::string time = timeInput->value();
            std::string reason = reasonInput->value();
            validateDate(date);
            validateTime(time);
            if (reason.empty()) throw ClinicException("Reason for the visit is required.");
            int id = m.scheduleAppointment(pid, date, time, reason);
            reasonInput->value("");
            notify("Appointment " + std::to_string(id) + " scheduled for " + date + " " + time);
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void onCancel() {
        if (selA == 0) { fl_alert("Error: select an appointment in the table first."); return; }
        try {
            m.cancelAppointment(selP, selA);
            notify("Appointment cancelled. Use Undo to restore it.");
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void onComplete() {
        if (selA == 0) { fl_alert("Error: select an appointment in the table first."); return; }
        try {
            m.completeAppointment(selP, selA);
            notify("Appointment marked as completed.");
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void onUndo() {
        if (!m.canUndo()) { fl_message("Nothing to undo."); return; }
        try {
            notify(m.undoLast());
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }
};
}
}
