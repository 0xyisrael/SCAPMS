#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <functional>
#include <string>
#include <vector>
#include "../controller/ClinicManager.h"

namespace clinic {
namespace gui {

class QueuePanel : public Fl_Group {
public:
    QueuePanel(int X, int Y, int W, int H, ClinicManager& mgr,
               std::function<void(const std::string&)> notifyFn)
        : Fl_Group(X, Y, W, H, "Waiting Queue"), m(mgr), notify(notifyFn)
    {
        box(FL_FLAT_BOX);

        // ---- Queue list ----
        browser = new Fl_Hold_Browser(X + 10, Y + 10, W - 400, H - 90);
        widths[0] = 45; widths[1] = 50; widths[2] = 190; widths[3] = 0;
        browser->column_widths(widths);
        browser->tooltip("Patients in the order they will be called");

        // ---- Now serving display ----
        servingBox = new Fl_Box(X + 10, Y + H - 70, W - 400, 60, "Now serving: nobody yet");
        servingBox->box(FL_DOWN_BOX);
        servingBox->labelsize(16);
        servingBox->labelfont(FL_BOLD);
        servingBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_WRAP);

        // ---- Controls ----
        form = new Fl_Pack(X + W - 280, Y + 10, 270, H - 20);
        form->type(Fl_Pack::VERTICAL);
        form->spacing(6);

        idInput = new Fl_Int_Input(0, 0, 270, 26, "Patient ID:");
        idInput->when(FL_WHEN_CHANGED);
        idInput->callback(idCb, this);
        idInput->tooltip("ID of the patient to add to the waiting queue");

        infoBox = new Fl_Box(0, 0, 270, 26, "");
        infoBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
        infoBox->labelcolor(FL_DARK_BLUE);

        Fl_Button* addBtn = new Fl_Button(0, 0, 270, 26, "Add to Queue");
        addBtn->callback(addCb, this);
        addBtn->tooltip("Put the patient at the back of the waiting queue");
        Fl_Button* nextBtn = new Fl_Button(0, 0, 270, 26, "Call Next Patient");
        nextBtn->callback(nextCb, this);
        nextBtn->tooltip("Call the patient who has waited longest (first in, first out)");

        countBox = new Fl_Box(0, 0, 270, 26, "Waiting: 0");
        countBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
        form->end();

        end();
        resizable(browser);
        refresh();
    }

    void refresh() {
        browser->clear();
        browser->add("@b@.Pos\t@b@.ID\t@b@.Name\t@b@.Category");
        int pos = 1;
        for (int id : m.waitingIds()) {
            PatientRecord* p = m.findPatient(id);
            if (!p) continue;
            std::string c = (p->getCategory() == "Emergency")
                                ? "@C" + std::to_string((int)FL_RED) + "@b@."   // emergencies stand out
                                : "@.";
            std::string line = c + std::to_string(pos++) + "\t" + c + std::to_string(id) + "\t" +
                               c + p->getName() + "\t" + c + p->getCategory();
            browser->add(line.c_str());
        }
        std::string t = "Waiting: " + std::to_string(m.waitingCount());
        countBox->copy_label(t.c_str());
        countBox->redraw();
    }

private:
    ClinicManager& m;
    std::function<void(const std::string&)> notify;
    Fl_Hold_Browser* browser;
    Fl_Box *servingBox, *infoBox, *countBox;
    Fl_Pack* form;
    Fl_Int_Input* idInput;
    int widths[4];

    static void idCb(Fl_Widget*, void* d)   { static_cast<QueuePanel*>(d)->showInfo(); }
    static void addCb(Fl_Widget*, void* d)  { static_cast<QueuePanel*>(d)->onAdd(); }
    static void nextCb(Fl_Widget*, void* d) { static_cast<QueuePanel*>(d)->onNext(); }

    void showInfo() {
        std::string s = idInput->value();
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

    void onAdd() {
        try {
            std::string s = idInput->value();
            if (s.empty()) throw ClinicException("Patient ID is required.");
            int id;
            try { id = std::stoi(s); }
            catch (...) { throw ClinicException("Patient ID must be a valid number."); }
            m.addToQueue(id);                // throws if not found or already waiting
            idInput->value("");
            infoBox->copy_label("");
            notify("Patient " + std::to_string(id) + " added to the waiting queue.");
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void onNext() {
        try {
            PatientRecord* p = m.callNext();
            if (!p) {
                servingBox->copy_label("Now serving: nobody yet");
                fl_message("No patients are waiting.");
                refresh();
                return;
            }
            std::string t = "Now serving: " + p->getName() + "\n(ID " + std::to_string(p->getId()) +
                            ", " + p->getCategory() + ")";
            servingBox->copy_label(t.c_str());
            notify("Called next patient: " + p->getName());
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }
};
}
}
