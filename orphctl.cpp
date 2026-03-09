#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <vector>

// ── Constants ────────────────────────────────────────────────────────────────
const std::string BINARY_DEST  = "/usr/local/bin/orphctl";
const std::string SERVICE_FILE = "/etc/systemd/system/orphctl.service";
const std::string TIMER_FILE   = "/etc/systemd/system/orphctl.timer";

// ── Helpers ──────────────────────────────────────────────────────────────────

bool isRoot() {
    return geteuid() == 0;
}

// Captures stdout of a shell command into a string
std::string runCommand(const std::string& cmd) {
    std::string result;
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return "";
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe))
        result += buffer;
    pclose(pipe);
    return result;
}

// Runs a command and streams output directly to the terminal
int runCommandLive(const std::string& cmd) {
    return system(cmd.c_str());
}

bool fileExists(const std::string& path) {
    struct stat st;
    return stat(path.c_str(), &st) == 0;
}

bool commandExists(const std::string& cmd) {
    return system(("command -v " + cmd + " > /dev/null 2>&1").c_str()) == 0;
}

bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error: could not write to " << path << "\n";
        return false;
    }
    file << content;
    return true;
}

// ── Self-elevation ────────────────────────────────────────────────────────────

// Relaunches this binary under sudo, forwarding all original args.
// Only called for commands that need root.
void elevate(int argc, char* argv[]) {
    if (isRoot()) return;

    std::cout << ">>> Elevation required — relaunching with sudo...\n";

    std::vector<char*> args;
    args.push_back(const_cast<char*>("sudo"));
    for (int i = 0; i < argc; i++)
        args.push_back(argv[i]);
    args.push_back(nullptr);

    execvp("sudo", args.data());

    // execvp only returns on failure
    std::cerr << "Error: failed to elevate. Try manually: sudo " << argv[0] << "\n";
    exit(1);
}

// ── Systemd automation setup ──────────────────────────────────────────────────

void installAutomation(const std::string& selfPath, const std::string& mode) {
    std::cout << ">>> First run — installing binary and systemd timer...\n";

    // Install binary
    runCommandLive("cp " + selfPath + " " + BINARY_DEST);
    runCommandLive("chmod +x " + BINARY_DEST);

    // systemd service — timer fires with the same mode arg used on first run
    std::string execStart = BINARY_DEST + (mode == "aur" ? " --aur" : "");
    writeFile(SERVICE_FILE,
        "[Unit]\n"
        "Description=orphctl - Remove orphaned packages and clean AUR cache\n"
        "\n"
        "[Service]\n"
        "Type=oneshot\n"
        "ExecStart=" + execStart + "\n"
    );

    // systemd timer — fires on the 1st and 15th of every month at 03:00
    // Persistent=true means if the machine was off, it catches up on next boot
    writeFile(TIMER_FILE,
        "[Unit]\n"
        "Description=Run orphctl cleanup every 2 weeks\n"
        "\n"
        "[Timer]\n"
        "OnCalendar=*-*-1,15 03:00:00\n"
        "Persistent=true\n"
        "\n"
        "[Install]\n"
        "WantedBy=timers.target\n"
    );

    // Reload systemd so it picks up the new files, then enable + start the timer
    runCommandLive("systemctl daemon-reload");
    runCommandLive("systemctl enable --now orphctl.timer");

    // Verify the timer actually started
    int ok = system("systemctl is-active orphctl.timer > /dev/null 2>&1");
    if (ok == 0)
        std::cout << "✔ Timer active. Cleanup runs on the 1st & 15th of each month at 03:00.\n\n";
    else
        std::cerr << "⚠ Timer may not have started. Check: systemctl status orphctl.timer\n\n";
}

// ── Cleanup actions ───────────────────────────────────────────────────────────

void runPacmanCleanup() {
    std::cout << "\n--- Pacman: Removing orphaned packages ---\n";
    runCommandLive("pacman -R $(pacman -Qdtq)");
    std::cout << "✔ Done.\n";
}

void runYayCacheCleanup() {
    std::cout << "\n--- Yay: Cleaning AUR cache ---\n";

    // yay -Scc must NOT run as root — drop back to the real user via SUDO_USER
    const char* sudoUser = getenv("SUDO_USER");
    if (sudoUser && std::string(sudoUser) != "root") {
        std::cout << "Running yay -Scc as user '" << sudoUser << "'...\n";
        runCommandLive(std::string("su - ") + sudoUser + " -c 'yay -Scc --noconfirm'");
    } else {
        runCommandLive("yay -Scc --noconfirm");
    }

    std::cout << "✔ Yay cache cleaned.\n";
}

void runCleanup(const std::string& selfPath, bool aurMode) {
    // Install binary + timer on first run
    if (!fileExists(BINARY_DEST))
        installAutomation(selfPath, aurMode ? "aur" : "pacman");

    if (aurMode) {
        // Check yay is available before doing anything
        if (!commandExists("yay")) {
            std::cerr << "Error: yay not found. Install yay or run orphctl without --aur.\n";
            exit(1);
        }
        std::cout << "=== orphctl — Cleanup (pacman + yay) ===\n";
        runPacmanCleanup();
        runYayCacheCleanup();
    } else {
        std::cout << "=== orphctl — Cleanup (pacman) ===\n";
        runPacmanCleanup();
    }

    std::cout << "\n✔ All done.\n";
}

// ── Other commands ────────────────────────────────────────────────────────────

void stopAutomation() {
    std::string check = runCommand("systemctl list-unit-files orphctl.timer 2>/dev/null");
    if (check.find("orphctl.timer") == std::string::npos) {
        std::cout << "Automation is not currently installed.\n";
        return;
    }

    std::cout << ">>> Disabling orphctl timer and removing all files...\n";
    runCommandLive("systemctl disable --now orphctl.timer 2>/dev/null");
    runCommandLive("rm -f " + SERVICE_FILE + " " + TIMER_FILE + " " + BINARY_DEST);
    runCommandLive("systemctl daemon-reload");
    std::cout << "✔ Automation removed.\n";
}

void showStats() {
    std::cout << "=== orphctl — Timer Status ===\n\n";

    int enabled = system("systemctl is-enabled orphctl.timer > /dev/null 2>&1");
    if (enabled != 0) {
        std::cout << "Automation is not enabled. Run: orphctl\n";
        return;
    }

    runCommandLive("systemctl status orphctl.timer --no-pager");
    std::cout << "\n--- Next scheduled run ---\n";
    runCommandLive("systemctl list-timers orphctl.timer --no-pager");
}

void showHelp(const std::string& name) {
    std::cout
        << "Usage: " << name << " [OPTION]\n\n"
        << "  (no args)   Remove orphaned packages via pacman.\n"
        << "              Sets up the systemd timer automatically on first run.\n"
        << "  --aur       Remove orphaned packages via pacman + clean yay cache.\n"
        << "              Timer will also run in this mode on future scheduled runs.\n"
        << "  --stop      Disable and remove the automatic cleanup timer.\n"
        << "  --stats     Show timer status and countdown to next scheduled run.\n"
        << "  --help      Show this help message.\n\n"
        << "Sudo elevation is handled automatically — no need to prefix with sudo.\n";
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    std::string selfPath = argv[0];
    std::string arg      = (argc > 1) ? argv[1] : "";

    // Commands that never need root
    if (arg == "--help" || arg == "-h") {
        showHelp(selfPath);
        return 0;
    }

    if (arg == "--stats") {
        showStats();
        return 0;
    }

    // Everything else needs root — elevate if not already
    elevate(argc, argv);

    // ── We are root from here ─────────────────────────────────────────────────

    if (arg == "--stop") {
        stopAutomation();
    } else if (arg == "--aur") {
        runCleanup(selfPath, true);
    } else if (arg.empty()) {
        runCleanup(selfPath, false);
    } else {
        std::cerr << "Unknown option: " << arg << "\n";
        std::cerr << "Run '" << selfPath << " --help' for usage.\n";
        return 1;
    }

    return 0;
}