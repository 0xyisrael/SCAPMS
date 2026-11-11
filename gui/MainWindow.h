#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>
#include <ctime>
#include <string>
#include "../controller/ClinicManager.h"
#include "../controller/ReportGenerator.h"
#include "../utils/FileHandler.h"
#include "PatientPanel.h"

namespace clinic {
namespace gui {

class MainWindow : public Fl_Double_Window {
public:
    explicit MainWindow(ClinicManager& mgr)
        : Fl_Double_Window(900, 600, "Smart Clinic Appointment & Patient Management System"),
          m(mgr)
    {
        // ---- Menu bar (menu events + keyboard shortcuts) ----
        menu = new Fl_Menu_Bar(0, 0, w(), 25);
        menu->add("&File/&New",  FL_CTRL + 'n', newCb,   this);
        menu->add("&File/&Save", FL_CTRL + 's', saveCb,  this);
        menu->add("&File/&Load", FL_CTRL + 'o', loadCb,  this);
        menu->add("&File/E&xit", FL_CTRL + 'q', exitCb,  this);
        menu->add("&Help/&About", 0,            aboutCb, this);

        // ---- Tabs ----
        tabs = new Fl_Tabs(0, 25, w(), h() - 50);

        Fl_Group* g = new Fl_Group(0, 50, w(), h() - 75, "Dashboard");
        dashboard = new Fl_Box(20, 60, w() - 40, h() - 95);
        dashboard->box(FL_DOWN_BOX);
        dashboard->labelsize(16);
        dashboard->align(FL_ALIGN_INSIDE | FL_ALIGN_TOP | FL_ALIGN_LEFT);
        g->resizable(dashboard);
        g->end();

        new PatientPanel(0, 50, w(), h() - 75, m, [this](const std::string& s) { setStatus(s); refresh(); });
        placeholder("Appointments",    "Appointments tab - coming in Step C");
        placeholder("Waiting Queue",   "Waiting Queue tab - coming in Step C");
        placeholder("Search && Sort",   "Search & Sort tab - coming in Step D");
        placeholder("Reports",         "Reports tab - coming in Step D");
        tabs->end();

        // ---- Status bar ----
        status = new Fl_Box(0, h() - 25, w(), 25, "Ready");
        status->box(FL_THIN_DOWN_BOX);
        status->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

        end();
        resizable(tabs);          // tabs grow/shrink when the window is resized
        size_range(700, 450);
        callback(exitCb, this);   // closing the window (X button or Esc) asks first

        refresh();
        Fl::add_timeout(5.0, timerCb, this);   // timer event
    }

    ~MainWindow() { Fl::remove_timeout(timerCb, this); }

    // Rebuilds the dashboard text from the current data
    void refresh() {
        std::string t = "CLINIC DASHBOARD\n\n";
        t += "Total patients: " + std::to_string(m.database().size()) + "\n";
        t += "Patients waiting: " + std::to_string(m.waitingCount()) + "\n\n";
        t += ReportGenerator::categoryDistribution(m) + "\n";
        t += ReportGenerator::dailySummary(m);
        dashboard->copy_label(t.c_str());
        dashboard->redraw();
    }

    void setStatus(const std::string& s) { status->copy_label(s.c_str()); }

private:
    ClinicManager& m;
    Fl_Menu_Bar* menu;
    Fl_Tabs* tabs;
    Fl_Box* dashboard;
    Fl_Box* status;

    void placeholder(const char* tabName, const char* text) {
        Fl_Group* g = new Fl_Group(0, 50, w(), h() - 75, tabName);
        Fl_Box* b = new Fl_Box(20, 60, w() - 40, h() - 95, text);
        b->labelsize(16);
        g->resizable(b);
        g->end();
    }

    static MainWindow* self(void* d) { return static_cast<MainWindow*>(d); }

    // ---- Event handlers (callbacks) ----
    static void timerCb(void* d) {
        MainWindow* w = self(d);
        w->refresh();
        char buf[16];
        std::time_t now = std::time(nullptr);
        std::strftime(buf, sizeof buf, "%H:%M:%S", std::localtime(&now));
        w->setStatus(std::string("Dashboard refreshed at ") + buf);
        Fl::repeat_timeout(5.0, timerCb, d);
    }

    static void newCb(Fl_Widget*, void* d) {
        MainWindow* w = self(d);
        if (fl_choice("Clear all patients and appointments?", "Cancel", "Clear", 0) == 1) {
            w->m.resetAll();
            w->refresh();
            w->setStatus("Started a new empty clinic.");
        }
    }

    static void saveCb(Fl_Widget*, void* d) {
        MainWindow* w = self(d);
        Fl_Native_File_Chooser fc;
        fc.title("Save clinic data");
        fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
        fc.filter("CSV Files\t*.csv");
        fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);
        if (fc.show() != 0) return;                       // user cancelled
        std::string path = fc.filename();
        if (path.size() < 4 || path.substr(path.size() - 4) != ".csv") path += ".csv";
        try {
            FileHandler::save(w->m, path);
            w->setStatus("Saved to " + path);
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    static void loadCb(Fl_Widget*, void* d) {
        MainWindow* w = self(d);
        Fl_Native_File_Chooser fc;
        fc.title("Load clinic data");
        fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
        fc.filter("CSV Files\t*.csv");
        if (fc.show() != 0) return;
        try {
            FileHandler::load(w->m, fc.filename());
            w->refresh();
            w->setStatus(std::string("Loaded ") + fc.filename());
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    static void exitCb(Fl_Widget*, void* d) {
        MainWindow* w = self(d);
        if (fl_choice("Exit SCAPMS?", "Cancel", "Exit", 0) == 1) w->hide();
    }

    static void aboutCb(Fl_Widget*, void*) {
        fl_message("Smart Clinic Appointment & Patient Management System\nCOS202 Lab Assessment");
    }
};
}
}
