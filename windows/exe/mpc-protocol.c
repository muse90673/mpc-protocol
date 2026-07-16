#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600

#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>
#include <time.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define WEBLINK_PREFIX L"mpc-hc://weblink?url="
#define LOG_FILE L"mpc-hc-protocol.log"
#define MAX_URL 2048

// Global variable: indicates whether the console was successfully attached
BOOL g_has_console = FALSE;

// Initialize console
void init_console() {
    // Try to attach to the parent process console that launched this program (e.g., CMD, PowerShell, MSYS2 Bash)
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        g_has_console = TRUE;
        
        // Redirect standard output (stdout) and standard error (stderr) to the attached console
        FILE* dummy;
        freopen_s(&dummy, "CONOUT$", "w", stdout);
        freopen_s(&dummy, "CONOUT$", "w", stderr);
        
        // Disable stdout buffering to ensure logs are immediately flushed to the terminal
        setvbuf(stdout, NULL, _IONBF, 0);
        setvbuf(stderr, NULL, _IONBF, 0);
    }
}

// Logging function
void log_message(const wchar_t* format, ...) {
    wchar_t timestamp[64];
    wchar_t message[4096];
    time_t now;
    struct tm* tm_info;
    va_list args;
    
    time(&now);
    tm_info = localtime(&now);
    wcsftime(timestamp, sizeof(timestamp)/sizeof(wchar_t), L"%Y-%m-%d %H:%M:%S", tm_info);
    
    va_start(args, format);
    _vsnwprintf(message, sizeof(message)/sizeof(wchar_t), format, args);
    va_end(args);
    
    // Write log to temp directory
    wchar_t log_path[MAX_PATH];
    GetTempPathW(MAX_PATH, log_path);
    wcscat(log_path, LOG_FILE);
    
    FILE* f = _wfopen(log_path, L"a");
    if (f) {
        // Note: %ls ensures wide characters are not truncated as single-byte
        fwprintf(f, L"[%ls] %ls\n", timestamp, message);
        fclose(f);
    }
    
    // If console was attached, output log directly to the terminal
    if (g_has_console) {
        wprintf(L"[%ls] %ls\n", timestamp, message);
        fflush(stdout);
    }
}

// URL decoding
wchar_t* decode_url(const wchar_t* encoded_url) {
    wchar_t* decoded = (wchar_t*)calloc(MAX_URL, sizeof(wchar_t));
    DWORD decoded_length = MAX_URL;
    
    if (!decoded) {
        log_message(L"Error: Failed to allocate memory for URL decoding");
        return NULL;
    }
    
    if (InternetCanonicalizeUrlW(encoded_url, decoded, &decoded_length, ICU_DECODE | ICU_NO_ENCODE)) {
        log_message(L"URL decoded successfully");
        return decoded;
    }
    
    DWORD error = GetLastError();
    log_message(L"Error: Failed to decode URL (error code: %lu)", error);
    free(decoded);
    return NULL;
}

// Fix broken http// or https// format caused by Chrome 130+ stripping slashes
wchar_t* fix_broken_url(wchar_t* url) {
    log_message(L"Fixing URL: %ls", url);
    
    if (wcsncmp(url, L"http//", 6) == 0) {
        log_message(L"Found broken http URL format");
        // Insert ":" into the original string
        size_t len = wcslen(url);
        memmove(url + 6, url + 5, (len - 5 + 1) * sizeof(wchar_t));  // +1 includes null terminator
        url[4] = L':';
        log_message(L"Fixed http URL: %ls", url);
    }
    else if (wcsncmp(url, L"https//", 7) == 0) {
        log_message(L"Found broken https URL format");
        // Insert ":" into the original string
        size_t len = wcslen(url);
        memmove(url + 7, url + 6, (len - 6 + 1) * sizeof(wchar_t));  // +1 includes null terminator
        url[5] = L':';
        log_message(L"Fixed https URL: %ls", url);
    }
    return url;
}

// Check if URL is valid
BOOL is_valid_url(const wchar_t* url) {
    return url && wcslen(url) > 0 && (wcsstr(url, L"http://") == url || wcsstr(url, L"https://") == url);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Initialize console attachment logic
    init_console();
    
    log_message(L"Starting MPC-HC Protocol Handler");
    log_message(L"Command line: %ls", pCmdLine);
    
    // Basic length check: mpc-hc:// (9 characters) plus minimum viable URL, must not be less than 12
    if (wcslen(pCmdLine) < 12) {
        log_message(L"Error: Command line too short");
        return 1;
    }

    // Remove possible surrounding double quotes from the argument
    if (pCmdLine[0] == '"') {
        pCmdLine++;
        if (pCmdLine[wcslen(pCmdLine)-1] == '"') {
            pCmdLine[wcslen(pCmdLine)-1] = 0;
        }
    }

    wchar_t* url = _wcsdup(pCmdLine);
    wchar_t* final_url = NULL;

    // 1. Remove mpc-hc:// protocol prefix (length 9)
    if (wcsncmp(url, L"mpc-hc://", 9) == 0) {
        wcscpy(url, url + 9);
        log_message(L"Removed mpc-hc:// prefix: %ls", url);
    }

    // 2. Parse weblink format (detect weblink?url= or weblink/?url=)
    if (wcsncmp(url, L"weblink?url=", 11) == 0 || wcsncmp(url, L"weblink/?url=", 12) == 0) {
        wchar_t* weblink_url = wcsstr(url, L"url=");
        if (weblink_url) {
            weblink_url += 4;  // Skip "url=" (4 characters)
            log_message(L"Extracting weblink URL: %ls", weblink_url);
            
            // Decode
            wchar_t* decoded = decode_url(weblink_url);
            if (!decoded) {
                log_message(L"Error: URL decoding failed");
                free(url);
                return 1;
            }
            
            final_url = decoded;
            log_message(L"Decoded URL: %ls", final_url);
        } else {
            log_message(L"Error: Invalid weblink format");
            free(url);
            return 1;
        }
        free(url);
    } else {
        // 3. Handle standard format and Chrome 130+ broken format (directly call fix function)
        final_url = fix_broken_url(url);
    }

    // Validate URL
    if (!is_valid_url(final_url)) {
        log_message(L"Error: Invalid URL: %ls", final_url);
        free(final_url);
        return 1;
    }

    // Encode spaces as +
    for (wchar_t* p = final_url; *p; p++) {
        if (*p == L' ') *p = L'+';
    }

    // 4. Locate MPC-HC (directly look for mpc-hc64.exe or mpc-hc.exe in the same directory)
    wchar_t self_path[MAX_PATH];
    GetModuleFileNameW(NULL, self_path, MAX_PATH);
    PathRemoveFileSpecW(self_path);  // Strip filename to get the directory of this program

    wchar_t mpc_path[MAX_PATH];
    // Prefer 64-bit program
    swprintf_s(mpc_path, MAX_PATH, L"%ls\\mpc-hc64.exe", self_path);
    if (!PathFileExistsW(mpc_path)) {
        // If not found, try matching 32-bit program
        swprintf_s(mpc_path, MAX_PATH, L"%ls\\mpc-hc.exe", self_path);
        if (!PathFileExistsW(mpc_path)) {
            log_message(L"Error: MPC-HC (mpc-hc64.exe / mpc-hc.exe) not found in current directory!");
            free(final_url);
            return 1;
        }
    }

    log_message(L"Found MPC-HC path: %ls", mpc_path);

    // 5. Assemble MPC-HC launch arguments: "[URL]" /play
    wchar_t* args = (wchar_t*)malloc(sizeof(wchar_t) * (wcslen(final_url) + 64));
    if (!args) {
        log_message(L"Error: Memory allocation failed");
        free(final_url);
        return 1;
    }
    wcscpy(args, L"\"");
    wcscat(args, final_url);
    wcscat(args, L"\" /play");

    log_message(L"Arguments: %ls", args);

    // Launch MPC-HC to start playback
    int ret = (INT_PTR)ShellExecute(NULL, NULL, mpc_path, args, NULL, SW_SHOWNORMAL);
    if (ret <= 32) {
        log_message(L"Error: Failed to start MPC-HC (error code: %d)", ret);
        free(args);
        free(final_url);
        return ret;
    }

    log_message(L"MPC-HC started successfully");
    free(args);
    free(final_url);
    
    // If console was attached, release it before exiting
    if (g_has_console) {
        FreeConsole();
    }
    return 0;
}
