#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/fl_ask.H>
#include <cctype>
#include <functional>
#include <string>
#include <vector>
#include "../controller/ClinicManager.h"
#include "../controller/SearchEngine.h"
#include "../controller/SortEngine.h"
#include "../utils/StringUtils.h"

namespace clinic {
namespace gui {

class SearchSortPanel : public Fl_Group {
public:
    SearchSortPanel(int X, int Y, int W, int H, ClinicManager& mgr,
                    std::function<void(const std::string&)> notifyFn)
        : Fl_Group(X, Y, W, H, "Search && Sort"), m(mgr), notify(notifyFn)
    {
        box(FL_FLAT_BOX);

        // ---- Results table ----
        browser = new Fl_Hold_Browser(X + 10, Y + 10, W - 400, H - 20);
        widths[0] = 50; widths[1] = 170; widths[2] = 50; widths[3] = 90; widths[4] = 0;
        browser->column_widths(widths);
        browser->tooltip("Search results (or all patients) appear here");

        // ---- Controls (Fl_Pack stacks them) ----
        form = new Fl_Pack(X + W - 280, Y + 10, 270, H - 20);
        form->type(Fl_Pack::VERTICAL);
        form->spacing(6);

        heading("Search Patients");

        byChoice = new Fl_Choice(0, 0, 270, 26, "Search by:");
        byChoice->add("Patient ID|Patient Name|Category");
        byChoice->value(0);
        byChoice->tooltip("Which field to search");

        methodChoice = new Fl_Choice(0, 0, 270, 26, "Method:");
        methodChoice->add("Automatic|Linear Search|Binary Search|Recursive Search");
        methodChoice->value(0);
        methodChoice->tooltip("Automatic: Binary Search if the list is sorted by ID, otherwise Linear Search. "
                              "Names use Recursive Search.");

        keywordInput = new Fl_Input(0, 0, 270, 26, "Keyword:");
        keywordInput->when(FL_WHEN_ENTER_KEY);        // text field event: Enter key searches
        keywordInput->callback(searchCb, this);
        keywordInput->tooltip("Type an ID, part of a name, or a category, then press Enter");

        Fl_Button* searchBtn = new Fl_Button(0, 0, 270, 26, "Search");
        searchBtn->callback(searchCb, this);
        searchBtn->tooltip("Search using the chosen field and method (Ctrl+F jumps here)");

        Fl_Button* allBtn = new Fl_Button(0, 0, 270, 26, "Show All Patients");
        allBtn->callback(showAllCb, this);
        allBtn->tooltip("Clear the search and list every patient");

        resultBox = new Fl_Box(0, 0, 270, 56, "");
        resultBox->labelsize(12);
        resultBox->labelcolor(FL_DARK_BLUE);
        resultBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_WRAP);

        heading("Sort Records");

        algoChoice = new Fl_Choice(0, 0, 270, 26, "Algorithm:");
        algoChoice->add("Insertion Sort|Merge Sort");
        algoChoice->value(0);
        algoChoice->callback(algoCb, this);
        algoChoice->tooltip("Choose which sorting algorithm to use");

        sortByChoice = new Fl_Choice(0, 0, 270, 26, "Sort by:");
        sortByChoice->add("Patient Name|Patient ID|Age");
        sortByChoice->value(0);
        sortByChoice->tooltip("Which field to sort the patient records by");

        Fl_Button* sortBtn = new Fl_Button(0, 0, 270, 26, "SORT");
        sortBtn->callback(sortCb, this);
        sortBtn->tooltip("Sort the patient records using the chosen algorithm");

        explainBox = new Fl_Box(0, 0, 270, 100, "");
        explainBox->labelsize(11);
        explainBox->box(FL_THIN_DOWN_BOX);
        explainBox->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_WRAP);
        form->end();

        end();
        resizable(browser);
        updateExplain();
        refresh();
    }

    // Shows every patient (in their current order)
    void refresh() {
        showList(m.database().getAll());
        std::string t = "Showing all " + std::to_string(m.database().size()) + " patient(s).";
        resultBox->copy_label(t.c_str());
    }

    // Used by the Ctrl+F shortcut
    void focusKeyword() { keywordInput->take_focus(); }

private:
    ClinicManager& m;
    std::function<void(const std::string&)> notify;
    Fl_Hold_Browser* browser;
    Fl_Pack* form;
    Fl_Choice *byChoice, *methodChoice, *algoChoice, *sortByChoice;
    Fl_Input* keywordInput;
    Fl_Box *resultBox, *explainBox;
    int widths[5];

    // ---------- helpers ----------
    static void heading(const char* text) {
        Fl_Box* b = new Fl_Box(0, 0, 270, 22, text);
        b->labelfont(FL_BOLD);
        b->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    }

    static std::string trim(const std::string& s) {
        size_t a = s.find_first_not_of(" \t");
        if (a == std::string::npos) return "";
        size_t b = s.find_last_not_of(" \t");
        return s.substr(a, b - a + 1);
    }

    static int parseId(const std::string& s) {
        for (char c : s)
            if (!std::isdigit(static_cast<unsigned char>(c)))
                throw ClinicException("Invalid search input: a Patient ID must contain digits only.");
        if (s.size() > 9) throw ClinicException("Invalid search input: that ID is too long.");
        return std::stoi(s);
    }

    // Binary search only works when the list is in ID order
    static bool isSortedById(const std::vector<PatientRecord*>& v) {
        for (size_t i = 1; i < v.size(); i++)
            if (v[i - 1]->getId() > v[i]->getId()) return false;
        return true;
    }

    // Custom presentation: colour depends on the patient category
    static std::string colourFor(const std::string& cat) {
        if (cat == "Emergency")
            return "@C" + std::to_string((int)FL_RED) + "@b@.";
        if (cat == "Follow-Up")
            return "@C" + std::to_string((int)FL_BLUE) + "@.";
        return "@.";
    }

    void showList(const std::vector<PatientRecord*>& list) {
        browser->clear();
        browser->add("@b@.ID\tName\tAge\tCategory\tContact");
        for (const PatientRecord* p : list) {
            std::string c = colourFor(p->getCategory());
            std::string line = c + std::to_string(p->getId()) + "\t" + c + p->getName() + "\t" +
                               c + std::to_string(p->getAge()) + "\t" + c + p->getCategory() + "\t" +
                               c + p->getContact();
            browser->add(line.c_str());
        }
    }

    // Explains why the chosen sorting algorithm fits the situation
    void updateExplain() {
        if (algoChoice->value() == 0)
            explainBox->copy_label(
                "Insertion Sort: simple and fast for small or nearly sorted lists. "
                "A clinic list that is kept in order as patients register needs only a few "
                "moves, so it suits everyday use.");
        else
            explainBox->copy_label(
                "Merge Sort: always takes about n log n steps, even for large or badly "
                "ordered lists. It suits big lists, for example hundreds of patients "
                "loaded from a file.");
        explainBox->redraw();
    }

    // ---------- event handlers ----------
    static void searchCb(Fl_Widget*, void* d)  { static_cast<SearchSortPanel*>(d)->onSearch(); }
    static void showAllCb(Fl_Widget*, void* d) { static_cast<SearchSortPanel*>(d)->onShowAll(); }
    static void sortCb(Fl_Widget*, void* d)    { static_cast<SearchSortPanel*>(d)->onSort(); }
    static void algoCb(Fl_Widget*, void* d)    { static_cast<SearchSortPanel*>(d)->updateExplain(); }

    void onShowAll() {
        keywordInput->value("");
        refresh();
    }

    void onSearch() {
        try {
            std::string key = trim(keywordInput->value());
            if (key.empty()) throw ClinicException("Please enter something to search for.");

            int by = byChoice->value();          // 0 = ID, 1 = Name, 2 = Category
            int method = methodChoice->value();  // 0 = Auto, 1 = Linear, 2 = Binary, 3 = Recursive
            const std::vector<PatientRecord*>& all = m.database().getAll();
            std::vector<PatientRecord*> found;
            std::string used;

            if (by == 0) {
                // ---- search by ID ----
                int id = parseId(key);
                bool sorted = isSortedById(all);
                if (method == 0) method = sorted ? 2 : 1;      // pick by the state of the data
                if (method == 3)
                    throw ClinicException("Recursive search is only available for names.");
                int idx;
                if (method == 2) {
                    if (!sorted)
                        throw ClinicException("Binary search needs the list sorted by ID.\n"
                                              "Sort by Patient ID first, or choose Linear Search.");
                    idx = SearchEngine::binarySearchById(all, id);
                    used = "Binary Search (list is sorted by ID)";
                } else {
                    idx = SearchEngine::linearSearchById(all, id);
                    used = "Linear Search (works on unsorted data)";
                }
                if (idx != -1) found.push_back(all[idx]);
            }
            else if (by == 1) {
                // ---- search by name ----
                if (method == 2)
                    throw ClinicException("Binary search is only available for Patient ID.");
                if (method == 0) method = 3;
                if (method == 3) {
                    std::vector<int> idxs;
                    SearchEngine::recursiveSearchByName(all, key, 0, idxs);
                    for (int i : idxs) found.push_back(all[i]);
                    used = "Recursive Search (name contains the keyword)";
                } else {
                    std::string k = toLower(key);
                    for (PatientRecord* p : all)
                        if (toLower(p->getName()).find(k) != std::string::npos) found.push_back(p);
                    used = "Linear Search (name contains the keyword)";
                }
            }
            else {
                // ---- search by category ----
                if (method == 2 || method == 3)
                    throw ClinicException("Category search supports Linear Search only.");
                std::string k = toLower(key);
                for (PatientRecord* p : all)
                    if (toLower(p->getCategory()).find(k) != std::string::npos) found.push_back(p);
                used = "Linear Search (category contains the keyword)";
            }

            showList(found);
            std::string msg = found.empty()
                                  ? std::string("No patient found.")
                                  : "Found " + std::to_string(found.size()) + " patient(s).";
            msg += "\nMethod: " + used;
            resultBox->copy_label(msg.c_str());
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }

    void onSort() {
        try {
            int algo = algoChoice->value();
            int by = sortByChoice->value();
            SortEngine::SortField field = SortEngine::SortField::Name;
            std::string fieldName = "Patient Name";
            if (by == 1) { field = SortEngine::SortField::Id;  fieldName = "Patient ID"; }
            if (by == 2) { field = SortEngine::SortField::Age; fieldName = "Age"; }

            if (algo == 0) SortEngine::insertionSort(m.database().getAll(), field);
            else           SortEngine::mergeSort(m.database().getAll(), field);

            keywordInput->value("");
            notify("Sorted by " + fieldName + " using " + (algo == 0 ? "Insertion Sort." : "Merge Sort."));
        } catch (const std::exception& e) {
            fl_alert("Error: %s", e.what());
        }
    }
};
}
}
