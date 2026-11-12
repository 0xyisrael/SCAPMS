#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <cctype>
#include <sstream>
#include <string>
#include "../controller/ClinicManager.h"
#include "../controller/ReportGenerator.h"

namespace clinic {
namespace gui {

class ReportPanel : public Fl_Group {
public:
    ReportPanel(int X, int Y, int W, int H, ClinicManager& mgr)
        : Fl_Group(X, Y, W, H, "Reports"), m(mgr)
    {
        box(FL_FLAT_BOX);

        // ---- Report display (fixed-width font keeps columns tidy) ----
        browser = new Fl_Browser(X + 10, Y + 10, W - 300, H - 20);
        browser->textfont(FL_COURIER);
        browser->textsize(14);

        // ---- Buttons ----
        form = new Fl_Pack(X + W - 280, Y + 10, 270, H - 20);
        form->type(Fl_Pack::VERTICAL);
        form->spacing(6);

        Fl_Button* allBtn = new Fl_Button(0, 0, 270, 26, "All Reports");
        allBtn->callback(allCb, this);
        allBtn->tooltip("Show all four reports together");
        Fl_Button* freqBtn = new Fl_Button(0, 0, 270, 26, "Most Frequently Attended");
        freqBtn->callback(freqCb, this);
        freqBtn->tooltip("Patients with the most completed appointments");
        Fl_Button* pendBtn = new Fl_Button(0, 0, 270, 26, "Patients with Pending Appointments");
        pendBtn->callback(pendCb, this);
        pendBtn->tooltip("Patients who still have appointments that are not completed");
        Fl_Button* catBtn = new Fl_Button(0, 0, 270, 26, "Patient Category Distribution");
        catBtn->callback(catCb, this);
        catBtn->tooltip("How many General, Emergency and Follow-Up patients");

        dateInput = new Fl_Input(0, 0, 270, 26, "Date:");
        dateInput->tooltip("YYYY-MM-DD for one day. Leave empty for all dates.");
        Fl_Button* dayBtn = new Fl_Button(0, 0, 270, 26, "Daily Appointment Summary");
        dayBtn->callback(dayCb, this);
        dayBtn->tooltip("Total, completed, pending and cancelled appointments");
        form->end();

        end();
        resizable(browser);
        render();
    }

    // Redraws whichever report is currently showing, using the latest data
    void refresh() { render(); }

private:
    ClinicManager& m;
    Fl_Browser* browser;
    Fl_Pack* form;
    Fl_Input* dateInput;
    int current = 0;               // 0 = all, 1..4 = single report
    std::string currentDate;       // date used by the daily summary

    static void allCb(Fl_Widget*, void* d)  { static_cast<ReportPanel*>(d)->onReport(0); }
    static void freqCb(Fl_Widget*, void* d) { static_cast<ReportPanel*>(d)->onReport(1); }
    static void pendCb(Fl_Widget*, void* d) { static_cast<ReportPanel*>(d)->onReport(2); }
    static void catCb(Fl_Widget*, void* d)  { static_cast<ReportPanel*>(d)->onReport(3); }
    static void dayCb(Fl_Widget*, void* d)  { static_cast<ReportPanel*>(d)->onReport(4); }

    static bool digits(const std::string& s) {
        for (char c : s) if (!std::isdigit(static_cast<unsigned char>(c))) return false;
        return true;
    }

    static void validateDate(const std::string& d) {
        if (d.size() != 10 || d[4] != '-' || d[7] != '-' || !digits(d.substr(0, 4)) ||
            !digits(d.substr(5, 2)) || !digits(d.substr(8, 2)))
            throw ClinicException("Date must be in the format YYYY-MM-DD, or leave it empty.");
    }

    void onReport(int which) {
        try {
            if (which == 4 || which == 0) {
                std::string d = dateInput->value();
                if (!d.empty()) validateDate(d);
                currentDate = d;
            }
            current = which;
            render();
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void render() {
        std::string text;
        switch (current) {
            case 1: text = ReportGenerator::mostFrequentPatients(m); break;
            case 2: text = ReportGenerator::pendingAppointments(m); break;
            case 3: text = ReportGenerator::categoryDistribution(m); break;
            case 4: text = ReportGenerator::dailySummary(m, currentDate); break;
            default:
                text = ReportGenerator::mostFrequentPatients(m) + "\n" +
                       ReportGenerator::pendingAppointments(m) + "\n" +
                       ReportGenerator::categoryDistribution(m) + "\n" +
                       ReportGenerator::dailySummary(m, currentDate);
        }
        browser->clear();
        std::istringstream in(text);
        std::string line;
        while (std::getline(in, line))
            browser->add(("@." + line).c_str());    // "@." stops names from being read as format codes
    }
};
}
}
