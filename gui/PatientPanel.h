#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <cstdint>
#include <functional>
#include <string>
#include "../controller/ClinicManager.h"

namespace clinic {
namespace gui {

class PatientPanel : public Fl_Group {
public:
    PatientPanel(int X, int Y, int W, int H, ClinicManager& mgr,
                 std::function<void(const std::string&)> notifyFn)
        : Fl_Group(X, Y, W, H, "Patient Records"), m(mgr), notify(notifyFn)
    {
        box(FL_FLAT_BOX);   // clears the area when fields appear/disappear

        // ---- Table of patients (custom colours per category) ----
        browser = new Fl_Hold_Browser(X + 10, Y + 10, W - 400, H - 20);
        widths[0] = 50; widths[1] = 170; widths[2] = 50; widths[3] = 90; widths[4] = 0;
        browser->column_widths(widths);
        browser->callback(browserCb, this);
        browser->tooltip("Click a patient to load it into the form");

        // ---- Form: Fl_Pack stacks the widgets vertically for us ----
        form = new Fl_Pack(X + W - 280, Y + 10, 270, H - 20);
        form->type(Fl_Pack::VERTICAL);
        form->spacing(6);

        typeChoice = new Fl_Choice(0, 0, 270, 26, "Type:");
        typeChoice->add("General|Emergency|Follow-Up");
        typeChoice->value(0);
        typeChoice->callback(typeCb, this);
        typeChoice->tooltip("Choose the patient type. Extra fields appear for some types.");

        idInput = new Fl_Int_Input(0, 0, 270, 26, "ID:");
        idInput->tooltip("Unique patient ID (numbers only)");
        nameInput = new Fl_Input(0, 0, 270, 26, "Name:");
        ageInput = new Fl_Int_Input(0, 0, 270, 26, "Age:");
        ageInput->tooltip("Age must be between 0 and 120");
        contactInput = new Fl_Input(0, 0, 270, 26, "Contact:");

        severityInput = new Fl_Int_Input(0, 0, 270, 26, "Severity:");
        severityInput->tooltip("Emergency severity from 1 (mild) to 5 (critical)");
        diagnosisInput = new Fl_Input(0, 0, 270, 26, "Diagnosis:");
        diagnosisInput->tooltip("Previous diagnosis for the follow-up visit");

        Fl_Button* addBtn = new Fl_Button(0, 0, 270, 26, "Add Patient");
        addBtn->callback(addCb, this);
        addBtn->tooltip("Register a new patient with the details above");
        Fl_Button* updBtn = new Fl_Button(0, 0, 270, 26, "Update Selected");
        updBtn->callback(updateCb, this);
        updBtn->tooltip("Save changes to the patient selected in the table");
        Fl_Button* delBtn = new Fl_Button(0, 0, 270, 26, "Delete Selected");
        delBtn->callback(deleteCb, this);
        delBtn->tooltip("Delete the selected patient (can be undone)");
        Fl_Button* undoBtn = new Fl_Button(0, 0, 270, 26, "Undo Last Admin Operation");
        undoBtn->callback(undoCb, this);
        undoBtn->tooltip("Reverse the last delete, edit or appointment cancellation");
        Fl_Button* clrBtn = new Fl_Button(0, 0, 270, 26, "Clear Form");
        clrBtn->callback(clearCb, this);
        clrBtn->tooltip("Empty the form so you can add a new patient");
        form->end();

        end();
        resizable(browser);
        updateType();
        refresh();
    }

    // Rebuilds the table from the database
    void refresh() {
        browser->clear();
        browser->add("@b@.ID\tName\tAge\tCategory\tContact", (void*)(intptr_t)0);
        std::string red = std::to_string((int)FL_RED);
        std::string blue = std::to_string((int)FL_BLUE);
        for (const PatientRecord* p : m.database().getAll()) {
            std::string prefix;
            std::string cat = p->getCategory();
            if (cat == "Emergency")      prefix = "@C" + red + "@b@.";
            else if (cat == "Follow-Up") prefix = "@C" + blue + "@.";
            else                         prefix = "@.";
            std::string line = prefix + std::to_string(p->getId()) + "\t" + p->getName() + "\t" +
                               std::to_string(p->getAge()) + "\t" + cat + "\t" + p->getContact();
            browser->add(line.c_str(), (void*)(intptr_t)p->getId());
        }
        // keep the same patient selected if still there
        bool found = false;
        for (int i = 2; i <= browser->size(); i++)
            if ((int)(intptr_t)browser->data(i) == selectedId) { browser->select(i); found = true; }
        if (!found) selectedId = 0;
    }

private:
    ClinicManager& m;
    std::function<void(const std::string&)> notify;
    Fl_Hold_Browser* browser;
    Fl_Pack* form;
    Fl_Choice* typeChoice;
    Fl_Int_Input *idInput, *ageInput, *severityInput;
    Fl_Input *nameInput, *contactInput, *diagnosisInput;
    int widths[5];
    int selectedId = 0;

    // ---------- helpers ----------
    static int readInt(Fl_Input* in, const std::string& what) {
        std::string s = in->value();
        if (s.empty()) throw ClinicException(what + " is required.");
        try { return std::stoi(s); }
        catch (...) { throw ClinicException(what + " must be a valid number."); }
    }

    // Dynamic components: show/hide fields depending on the patient type
    void updateType() {
        int t = typeChoice->value();
        if (t == 1) severityInput->show();  else severityInput->hide();
        if (t == 2) diagnosisInput->show(); else diagnosisInput->hide();
        form->redraw();
        redraw();
    }

    void clearForm() {
        idInput->value(""); idInput->readonly(0);
        nameInput->value(""); ageInput->value(""); contactInput->value("");
        severityInput->value(""); diagnosisInput->value("");
        selectedId = 0;
        browser->deselect();
    }

    // ---------- event handlers ----------
    static void typeCb(Fl_Widget*, void* d) { static_cast<PatientPanel*>(d)->updateType(); }
    static void browserCb(Fl_Widget*, void* d) { static_cast<PatientPanel*>(d)->onSelect(); }
    static void addCb(Fl_Widget*, void* d) { static_cast<PatientPanel*>(d)->onAdd(); }
    static void updateCb(Fl_Widget*, void* d) { static_cast<PatientPanel*>(d)->onUpdate(); }
    static void deleteCb(Fl_Widget*, void* d) { static_cast<PatientPanel*>(d)->onDelete(); }
    static void undoCb(Fl_Widget*, void* d) { static_cast<PatientPanel*>(d)->onUndo(); }
    static void clearCb(Fl_Widget*, void* d) { static_cast<PatientPanel*>(d)->clearForm(); }

    void onSelect() {
        int line = browser->value();
        if (line <= 0) return;
        int id = (int)(intptr_t)browser->data(line);
        if (id == 0) { browser->deselect(); return; }      // header row
        PatientRecord* p = m.findPatient(id);
        if (!p) return;
        selectedId = id;
        idInput->value(std::to_string(id).c_str());
        idInput->readonly(1);                              // ID cannot be changed
        nameInput->value(p->getName().c_str());
        ageInput->value(std::to_string(p->getAge()).c_str());
        contactInput->value(p->getContact().c_str());
        severityInput->value("");
        diagnosisInput->value("");
        std::string c = p->getCategory();
        typeChoice->value(c == "General" ? 0 : (c == "Emergency" ? 1 : 2));
        if (auto e = dynamic_cast<EmergencyPatient*>(p))
            severityInput->value(std::to_string(e->getSeverity()).c_str());
        if (auto f = dynamic_cast<FollowUpPatient*>(p))
            diagnosisInput->value(f->getPreviousDiagnosis().c_str());
        updateType();
    }

    void onAdd() {
        try {
            int id = readInt(idInput, "Patient ID");
            if (id <= 0) throw ClinicException("Patient ID must be greater than 0.");
            std::string name = nameInput->value();
            int age = readInt(ageInput, "Patient age");
            std::string contact = contactInput->value();

            PatientRecord* p = nullptr;
            int t = typeChoice->value();
            if (t == 0) {
                p = new GeneralPatient(id, name, age, contact);
            } else if (t == 1) {
                p = new EmergencyPatient(id, name, age, contact, readInt(severityInput, "Severity"));
            } else {
                std::string diag = diagnosisInput->value();
                if (diag.empty()) throw ClinicException("Previous diagnosis is required.");
                p = new FollowUpPatient(id, name, age, contact, diag);
            }
            m.addPatient(p);                 // throws if the ID already exists
            clearForm();
            refresh();
            notify("Patient added: " + name);
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void onUpdate() {
        if (selectedId == 0) { fl_alert("Error: select a patient in the table first."); return; }
        try {
            PatientRecord* p = m.findPatient(selectedId);
            if (!p) throw ClinicException("Patient not found.");
            int t = typeChoice->value();
            std::string cat = p->getCategory();
            int actual = (cat == "General") ? 0 : (cat == "Emergency" ? 1 : 2);
            if (t != actual)
                throw ClinicException("Patient type cannot be changed. Delete and re-add the patient.");

            int age = readInt(ageInput, "Patient age");
            int severity = 0;
            if (actual == 1) {
                severity = readInt(severityInput, "Severity");
                if (severity < 1 || severity > 5)
                    throw ClinicException("Severity must be between 1 and 5.");
            }
            std::string diag = diagnosisInput->value();
            if (actual == 2 && diag.empty())
                throw ClinicException("Previous diagnosis is required.");

            m.editPatient(selectedId, nameInput->value(), age, contactInput->value());
            if (auto e = dynamic_cast<EmergencyPatient*>(p)) e->setSeverity(severity);
            if (auto f = dynamic_cast<FollowUpPatient*>(p)) f->setPreviousDiagnosis(diag);
            refresh();
            notify("Patient updated.");
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void onDelete() {
        if (selectedId == 0) { fl_alert("Error: select a patient in the table first."); return; }
        if (fl_choice("Delete this patient?\n(You can undo it afterwards.)", "Cancel", "Delete", 0) != 1)
            return;
        try {
            m.deletePatient(selectedId);
            clearForm();
            refresh();
            notify("Patient deleted. Use Undo to restore.");
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void onUndo() {
        if (!m.canUndo()) { fl_message("Nothing to undo."); return; }
        try {
            std::string msg = m.undoLast();
            refresh();
            notify(msg);
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }
};
}
}
