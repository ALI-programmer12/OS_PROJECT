#include <iostream>
#include <vector>
#include <limits>
#include <string>
#include <tuple>
#include <algorithm>
#include <raylib.h>
#include "process.h"
#include "simulator.h"
#include "ranker.h"

using namespace std;

// ---------------------------------------------------------------------------
// Global Constants and UI Colors
// ---------------------------------------------------------------------------
static const int SW  = 1400;
static const int SH  = 800;
static const int PAD = 48;

static const Color C_BG      = {  10,  13,  22, 255 };
static const Color C_CARD    = {  20,  25,  42, 255 };
static const Color C_CARD2   = {  28,  34,  56, 255 };
static const Color C_BORDER  = {  55,  66,  98, 255 };
static const Color C_TEXT    = { 230, 235, 250, 255 };
static const Color C_MUTED   = { 130, 142, 170, 255 };
static const Color C_ACCENT  = {  99, 162, 255, 255 };
static const Color C_ACCENT2 = {  72, 199, 142, 255 };
static const Color C_WARN    = { 255, 196,  80, 255 };
static const Color C_DANGER  = { 255, 100, 120, 255 };

static const Color PROC_COL[] = {
    {  99, 162, 255, 255 }, { 255, 154,  86, 255 },
    {  82, 210, 140, 255 }, { 210,  98, 255, 255 },
    { 255,  88, 116, 255 }, { 255, 210,  90, 255 },
};
static const int PALETTE_SZ = 6;

// ---------------------------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------------------------
static Color procColor(int pid) {
    if (pid <= 0) return (Color){ 60, 60, 70, 255 };
    return PROC_COL[(pid - 1) % PALETTE_SZ];
}

static void Separator(int y) {
    DrawLine(PAD, y, SW - PAD, y, C_BORDER);
}

static void DrawCard(Rectangle r, Color fill, Color border) {
    DrawRectangleRounded(r, 0.12f, 10, fill);
    DrawRectangleRoundedLines(r, 0.12f, 10, border);
}

static bool Hovered(Rectangle r) {
    return CheckCollisionPointRec(GetMousePosition(), r);
}

static bool Button(Rectangle b, const char *label, bool active = false,
                   Color accentOverride = {0,0,0,0}) {
    bool hov     = Hovered(b);
    bool clicked = hov && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
    Color accent = (accentOverride.a > 0) ? accentOverride : C_ACCENT;
    Color fill   = active ? (Color){40,100,200,255} : (hov ? (Color){32,40,66,255} : C_CARD);
    Color border = (active || hov) ? accent : C_BORDER;
    Color text   = active ? RAYWHITE : (hov ? C_TEXT : C_MUTED);
    DrawRectangleRounded(b, 0.20f, 10, fill);
    DrawRectangleRoundedLines(b, 0.20f, 10, border);
    DrawText(label, b.x + (b.width - MeasureText(label, 18)) / 2,
             b.y + (b.height - 18) / 2, 18, text);
    return clicked;
}

static void TextCentre(const char *s, int y, int sz, Color col) {
    DrawText(s, (SW - MeasureText(s, sz)) / 2, y, sz, col);
}

static void InputField(Rectangle r, const char *label, const string &val,
                       bool active, bool labelAbove = true) {
    int labelY = labelAbove ? (int)(r.y - 24) : (int)(r.y + (r.height - 18) / 2);
    DrawText(label, (int)r.x, labelY, 17, active ? C_ACCENT : C_MUTED);
    DrawRectangleRounded(r, 0.15f, 8, active ? (Color){25,40,75,255} : C_CARD);
    DrawRectangleRoundedLines(r, 0.15f, 8, active ? C_ACCENT : C_BORDER);
    string display = val + (active && ((int)(GetTime() * 2) % 2 == 0) ? "|" : "");
    DrawText(display.c_str(), (int)r.x + 10,
             (int)(r.y + (r.height - 18) / 2), 18, C_TEXT);
}

// ---------------------------------------------------------------------------
// Gantt Chart — draws merged segments, handles idle (pid=0), scrollable
// ---------------------------------------------------------------------------
static void DrawGanttChart(const vector<tuple<int,int,int>> &gantt,
                           int x, int y, int maxW) {
    if (gantt.empty()) return;

    int totalTime = 0;
    for (auto &[s, pid, d] : gantt)
        totalTime = max(totalTime, s + d);
    if (totalTime == 0) return;

    float scale = (float)maxW / (float)totalTime;
    const int BAR_H = 40;

    for (auto &[start, pid, dur] : gantt) {
        int bx = x + (int)(start * scale);
        int bw = max(2, (int)(dur * scale));   // minimum 2px so it's always visible

        Color fill = (pid > 0)
            ? ColorAlpha(procColor(pid), 0.75f)
            : (Color){50, 50, 60, 200};        // idle = dark grey

        DrawRectangle(bx, y, bw, BAR_H, fill);
        DrawRectangleLines(bx, y, bw, BAR_H, C_BORDER);

        // Label: only if there's room
        if (bw > 28) {
            const char *lbl = (pid > 0)
                ? TextFormat("P%d", pid)
                : "Idle";
            DrawText(lbl,
                     bx + (bw - MeasureText(lbl, 15)) / 2,
                     y + (BAR_H - 15) / 2,
                     15, RAYWHITE);
        }

        // Time tick below
        DrawText(TextFormat("%d", start), bx, y + BAR_H + 5, 12, C_MUTED);
    }

    // Final end-time tick
    int lastX = x + (int)(totalTime * scale);
    DrawText(TextFormat("%d", totalTime), lastX - 6, y + BAR_H + 5, 12, C_MUTED);
}

// ---------------------------------------------------------------------------
// State machine
// ---------------------------------------------------------------------------
enum State {
    INPUT_NUM, INPUT_PRIORITY, INPUT_PROCESSES, INPUT_QUANTUM,
    SELECT_ALGO, VIEW_ALGO, COMPARE
};

struct ProcInput {
    int at=0, bt=0, pr=0;
    string at_s="", bt_s="", pr_s="";
};

// ---------------------------------------------------------------------------
// Algorithm table — algoId mapping matches simulator.cpp switch:
//   1=FCFS  2=RR  3=SJF  4=SRTF  5=LRTF  6=MLFQ  7=Pre-Priority  8=NPPriority
// ---------------------------------------------------------------------------
static const int   ALGO_IDS[]   = { 1, 3, 2, 4, 5, 8, 7, 6, -1 };
static const char* ALGO_NAMES[] = {
    "FCFS",
    "SJF (Non-Pre)",
    "Round Robin",
    "SRTF",
    "LRTF",
    "Priority (Non-Pre)",
    "Priority (Pre)",
    "MLFQ",
    "\xe2\x9a\xa1 Compare All"
};
static const int ALGO_COUNT = 9;   // includes Compare All

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------
int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI);
    InitWindow(SW, SH, "CPU Scheduling Simulator");
    InitAudioDevice();
    SetTargetFPS(60);

    Sound bgSound = FileExists("audios/a2.wav") ? LoadSound("audios/a2.wav") : (Sound){0};
    if (bgSound.frameCount > 0) PlaySound(bgSound);
    Texture2D bgTexture = FileExists("images/bg.jpg")
        ? LoadTexture("images/bg.jpg") : (Texture2D){0};

    State  state      = INPUT_NUM;
    int    numProc    = 0;
    string numStr     = "", qStr = "4";
    bool   hasPriority = false;
    int    curProc    = 0, activeField = 0;
    vector<ProcInput> procInputs;
    Process *p = nullptr;

    while (!WindowShouldClose()) {

        // ----------------------------------------------------------------
        // INPUT HANDLING
        // ----------------------------------------------------------------
        int key = GetKeyPressed();
        if (key == KEY_TAB)
            activeField = (activeField + 1) % (hasPriority ? 3 : 2);

        if (IsKeyPressed(KEY_BACKSPACE)) {
            auto del = [](string &s){ if (!s.empty()) s.pop_back(); };
            if (state == INPUT_NUM)      del(numStr);
            else if (state == INPUT_QUANTUM) del(qStr);
            else if (state == INPUT_PROCESSES) {
                if (activeField == 0)      del(procInputs[curProc].at_s);
                else if (activeField == 1) del(procInputs[curProc].bt_s);
                else if (activeField == 2) del(procInputs[curProc].pr_s);
            }
        }

        if (key >= 48 && key <= 57) {
            char c = (char)key;
            if (state == INPUT_NUM)          numStr += c;
            else if (state == INPUT_QUANTUM)  qStr   += c;
            else if (state == INPUT_PROCESSES) {
                if (activeField == 0)          procInputs[curProc].at_s += c;
                else if (activeField == 1)     procInputs[curProc].bt_s += c;
                else if (activeField == 2)     procInputs[curProc].pr_s += c;
            }
        }

        // ----------------------------------------------------------------
        // DRAWING
        // ----------------------------------------------------------------
        BeginDrawing();
        ClearBackground(C_BG);

        if (bgTexture.id > 0) {
            DrawTexturePro(bgTexture,
                {0,0,(float)bgTexture.width,(float)bgTexture.height},
                {0,0,(float)SW,(float)SH}, {0,0}, 0, ColorAlpha(WHITE, 0.15f));
        }

        // Header bar
        DrawRectangle(0, 0, SW, 56, C_CARD);
        TextCentre("CPU Scheduling Simulator", 16, 26, C_TEXT);

        // ---- Step 1: Number of processes ----
        if (state == INPUT_NUM) {
            TextCentre("Step 1: System Config", 100, 20, C_ACCENT);
            InputField({SW/2-150, 200, 300, 50}, "How many processes?", numStr, true);
            if (Button({SW/2-80, 280, 160, 40}, "Continue") && !numStr.empty()) {
                numProc = stoi(numStr);
                numProc = min(numProc, 10);  // Cap to prevent long simulations
                procInputs.resize(numProc);
                state = INPUT_PRIORITY;
            }
        }

        // ---- Step 2: Priority yes/no ----
        else if (state == INPUT_PRIORITY) {
            TextCentre("Do processes have priorities?", 200, 22, C_TEXT);
            if (Button({SW/2-170, 260, 160, 50}, "Yes"))
                { hasPriority = true;  state = INPUT_PROCESSES; }
            if (Button({SW/2+10,  260, 160, 50}, "No"))
                { hasPriority = false; state = INPUT_PROCESSES; }
        }

        // ---- Step 3: Per-process data entry ----
        else if (state == INPUT_PROCESSES) {
            TextCentre(TextFormat("Data Entry: Process %d / %d", curProc+1, numProc),
                       100, 22, C_TEXT);
            InputField({SW/2-210, 200, 200, 50}, "Arrival Time",
                       procInputs[curProc].at_s, activeField == 0);
            InputField({SW/2+10,  200, 200, 50}, "Burst Time",
                       procInputs[curProc].bt_s, activeField == 1);
            if (hasPriority)
                InputField({SW/2-100, 290, 200, 50}, "Priority Value",
                           procInputs[curProc].pr_s, activeField == 2);

            DrawText("Press TAB to switch fields", SW/2-100, SH-100, 16, C_MUTED);

            if (Button({SW/2-80, 380, 160, 40},
                       curProc < numProc-1 ? "Next Process" : "Finalize")) {
                procInputs[curProc].at =
                    procInputs[curProc].at_s.empty() ? 0 : stoi(procInputs[curProc].at_s);
                procInputs[curProc].bt =
                    procInputs[curProc].bt_s.empty() ? 1 : stoi(procInputs[curProc].bt_s);
                if (hasPriority)
                    procInputs[curProc].pr =
                        procInputs[curProc].pr_s.empty() ? 0 : stoi(procInputs[curProc].pr_s);

                // Cap values to prevent excessive simulation time
                procInputs[curProc].at = min(procInputs[curProc].at, 100);
                procInputs[curProc].bt = max(1, min(procInputs[curProc].bt, 100));
                if (hasPriority)
                    procInputs[curProc].pr = min(procInputs[curProc].pr, 100);

                if (++curProc >= numProc) state = INPUT_QUANTUM;
                else activeField = 0;
            }
        }

        // ---- Step 4: Quantum ----
        else if (state == INPUT_QUANTUM) {
            TextCentre("Step 4: Time Quantum", 100, 20, C_ACCENT);
            InputField({SW/2-150, 200, 300, 50},
                       "Time Quantum (for RR / MLFQ)", qStr, true);
            if (Button({SW/2-80, 280, 160, 40}, "Run Engine")) {
                if (qStr.empty()) qStr = "4";
                if (p) delete[] p;
                p = new Process[numProc];
                for (int i = 0; i < numProc; i++) {
                    p[i] = { i+1,
                              procInputs[i].at,
                              procInputs[i].bt,
                              procInputs[i].bt,
                              hasPriority ? procInputs[i].pr : 0, 0, 0 };
                }
                state = SELECT_ALGO;
            }
        }

        // ---- Algorithm selection (8 buttons in 2 columns) ----
        else if (state == SELECT_ALGO) {
            TextCentre("Choose Algorithm to Analyze", 75, 24, C_TEXT);

            const float BTN_W = 290, BTN_H = 50, GAP = 14;
            const float COL0  = SW/2 - BTN_W - GAP/2;
            const float COL1  = SW/2 + GAP/2;
            float startY = 130;

            // 7 algo buttons in 2 columns, Compare All full-width at bottom
            for (int i = 0; i < ALGO_COUNT - 1; i++) {
                float bx = (i % 2 == 0) ? COL0 : COL1;
                float by = startY + (i / 2) * (BTN_H + GAP);
                bool isPrio = (ALGO_IDS[i] == 5 || ALGO_IDS[i] == 7);
                Color accent = isPrio ? C_ACCENT2 : C_ACCENT;
                if (Button({bx, by, BTN_W, BTN_H}, ALGO_NAMES[i], false, accent)) {
                    results.clear();
                    int q = qStr.empty() ? 4 : stoi(qStr);
                    simulate(p, numProc, ALGO_NAMES[i], ALGO_IDS[i], q);
                    state = VIEW_ALGO;
                }
            }

            // "Compare All" full-width button
            int rows = (ALGO_COUNT - 1 + 1) / 2;  // ceil((ALGO_COUNT-1)/2)
            float compareY = startY + rows * (BTN_H + GAP) + GAP;
            if (Button({SW/2 - 200, compareY, 400, BTN_H},
                       ALGO_NAMES[ALGO_COUNT-1], false, C_WARN)) {
                results.clear();
                int q = qStr.empty() ? 4 : stoi(qStr);
                for (int i = 0; i < ALGO_COUNT - 1; i++)
                    simulate(p, numProc, ALGO_NAMES[i], ALGO_IDS[i], q);
                rankAlgorithms();
                state = COMPARE;
            }
        }

        // ---- Single algorithm result view ----
        else if (state == VIEW_ALGO && !results.empty()) {
            Result &r = results.back();
            TextCentre(TextFormat("Analysis: %s", r.name.c_str()), 70, 24, C_ACCENT2);
            Separator(115);

            // Stats
            DrawText(TextFormat("Avg Waiting Time: %.2f    |    Avg Turnaround Time: %.2f",
                                r.avgWT, r.avgTAT),
                     PAD, 130, 20, C_TEXT);

            // Gantt chart
            DrawText("Gantt Chart", PAD, 185, 18, C_MUTED);
            DrawGanttChart(r.gantt, PAD, 215, SW - PAD*2);

            // Process table
            Separator(310);
            DrawText("PID", PAD,      330, 17, C_MUTED);
            DrawText("Arrival",  PAD+80,  330, 17, C_MUTED);
            DrawText("Burst",    PAD+180, 330, 17, C_MUTED);
            DrawText("Priority", PAD+280, 330, 17, C_MUTED);
            DrawText("WT",       PAD+380, 330, 17, C_MUTED);
            DrawText("TAT",      PAD+460, 330, 17, C_MUTED);

            const auto &displayProcs = r.procs;
            if (!displayProcs.empty()) {
                for (int i = 0; i < (int)displayProcs.size(); i++) {
                    int ry = 360 + i * 32;
                    Color rc = (i % 2 == 0) ? C_CARD : C_CARD2;
                    DrawRectangle(PAD, ry, SW - PAD*2, 28, rc);
                    DrawText(TextFormat("P%d",   displayProcs[i].pid), PAD,      ry+6, 16, procColor(displayProcs[i].pid));
                    DrawText(TextFormat("%d",    displayProcs[i].at),  PAD+80,   ry+6, 16, C_TEXT);
                    DrawText(TextFormat("%d",    displayProcs[i].bt),  PAD+180,  ry+6, 16, C_TEXT);
                    DrawText(TextFormat("%d",    displayProcs[i].pr),  PAD+280,  ry+6, 16, C_TEXT);
                    DrawText(TextFormat("%d",    displayProcs[i].wt),  PAD+380,  ry+6, 16, C_ACCENT2);
                    DrawText(TextFormat("%d",    displayProcs[i].tat), PAD+460,  ry+6, 16, C_WARN);
                }
            }

            if (Button({PAD, SH-72, 130, 40}, "\xe2\x86\x90 Back")) state = SELECT_ALGO;
        }

        // ---- Compare All results ----
        else if (state == COMPARE) {
            TextCentre("Algorithm Ranking (Best → Worst Avg WT)", 70, 22, C_WARN);
            Separator(110);

            // Column headers
            DrawText("Rank", PAD+10,     125, 16, C_MUTED);
            DrawText("Algorithm",  PAD+80,    125, 16, C_MUTED);
            DrawText("Avg WT",     PAD+400,   125, 16, C_MUTED);
            DrawText("Avg TAT",    PAD+540,   125, 16, C_MUTED);

            for (int i = 0; i < (int)results.size(); i++) {
                Rectangle row = {PAD, (float)(150 + i*50), SW-PAD*2, 42};
                Color cardCol = (i == 0) ? (Color){20,60,30,200} :
                                (i == 1) ? (Color){40,40,10,200} : C_CARD;
                DrawCard(row, cardCol, i == 0 ? C_ACCENT2 : C_BORDER);

                DrawText(TextFormat("#%d", i+1), PAD+14, 162+i*50, 18,
                         i==0 ? C_ACCENT2 : C_MUTED);
                DrawText(results[i].name.c_str(), PAD+80,  162+i*50, 18, C_TEXT);
                DrawText(TextFormat("%.2f",  results[i].avgWT),  PAD+400, 162+i*50, 18, C_ACCENT);
                DrawText(TextFormat("%.2f",  results[i].avgTAT), PAD+540, 162+i*50, 18, C_WARN);

                // Click row to view Gantt for that algorithm
                if (Hovered(row) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    // Bring clicked result to results.back() by rotating
                    Result clicked = results[i];
                    results.erase(results.begin() + i);
                    results.push_back(clicked);
                    state = VIEW_ALGO;
                }
            }

            DrawText("(Click a row to view its Gantt chart)",
                     PAD, SH-110, 15, C_MUTED);
            if (Button({PAD, SH-72, 130, 40}, "\xe2\x86\x90 Back")) state = SELECT_ALGO;
        }

        EndDrawing();
    }

    if (p) delete[] p;
    UnloadTexture(bgTexture);
    UnloadSound(bgSound);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
