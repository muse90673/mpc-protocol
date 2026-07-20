#define _WIN32_WINNT 0x0600
#define _WIN32_IE 0x0600

#include <windows.h>
#include <shlwapi.h>
#include <wininet.h>
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define LOG_FILE L"mpc-hc-protocol.log"

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
    struct tm tm_info;
    va_list args;

    if (time(&now) == (time_t)-1 ||
        localtime_s(&tm_info, &now) != 0 ||
        wcsftime(timestamp, ARRAY_SIZE(timestamp), L"%Y-%m-%d %H:%M:%S", &tm_info) == 0) {
        return;
    }

    va_start(args, format);
    _vsnwprintf_s(message, ARRAY_SIZE(message), _TRUNCATE, format, args);
    va_end(args);

    // Write log to temp directory
    wchar_t log_path[MAX_PATH];
    DWORD temp_length = GetTempPathW(ARRAY_SIZE(log_path), log_path);
    if (temp_length > 0 && temp_length < ARRAY_SIZE(log_path) &&
        temp_length <= ARRAY_SIZE(log_path) - ARRAY_SIZE(LOG_FILE)) {
        memcpy(log_path + temp_length, LOG_FILE, ARRAY_SIZE(LOG_FILE) * sizeof(*log_path));

        FILE* f = _wfopen(log_path, L"a");
        if (f) {
            // Note: %ls ensures wide characters are not truncated as single-byte
            fwprintf(f, L"[%ls] %ls\n", timestamp, message);
            fclose(f);
        }
    }

    // If console was attached, output log directly to the terminal
    if (g_has_console) {
        wprintf(L"[%ls] %ls\n", timestamp, message);
        fflush(stdout);
    }
}

// URL decoding
wchar_t* decode_url(const wchar_t* encoded_url) {
    if (!encoded_url) {
        return NULL;
    }

    size_t encoded_length = wcslen(encoded_url);
    if (encoded_length >= UINT32_MAX) {
        log_message(L"Error: URL is too long to decode");
        return NULL;
    }

    DWORD decoded_length = (DWORD)encoded_length + 1;
    wchar_t* decoded = (wchar_t*)calloc(decoded_length, sizeof(*decoded));

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
    const wchar_t* protocol = NULL;
    const wchar_t* remainder = NULL;

    if (!url) {
        return NULL;
    }

    log_message(L"Fixing URL: %ls", url);

    if (wcsncmp(url, L"http//", 6) == 0) {
        log_message(L"Found broken http URL format");
        protocol = L"http://";
        remainder = url + 6;
    } else if (wcsncmp(url, L"https//", 7) == 0) {
        log_message(L"Found broken https URL format");
        protocol = L"https://";
        remainder = url + 7;
    }

    if (!protocol) {
        return url;
    }

    size_t protocol_length = wcslen(protocol);
    size_t remainder_length = wcslen(remainder);
    if (remainder_length > (SIZE_MAX / sizeof(wchar_t)) - protocol_length - 1) {
        log_message(L"Error: URL is too long to repair");
        free(url);
        return NULL;
    }

    wchar_t* fixed = (wchar_t*)malloc((protocol_length + remainder_length + 1) * sizeof(*fixed));
    if (!fixed) {
        log_message(L"Error: Memory allocation failed while fixing URL");
        free(url);
        return NULL;
    }

    memcpy(fixed, protocol, protocol_length * sizeof(*fixed));
    memcpy(fixed + protocol_length, remainder, (remainder_length + 1) * sizeof(*fixed));
    free(url);

    log_message(L"Fixed URL: %ls", fixed);
    return fixed;
}

wchar_t* escape_url_spaces(const wchar_t* url) {
    if (!url) {
        return NULL;
    }

    size_t length = wcslen(url);
    size_t space_count = 0;
    for (const wchar_t* p = url; *p; p++) {
        if (*p == L' ') {
            space_count++;
        }
    }

    if (space_count > (SIZE_MAX - length - 1) / 2) {
        return NULL;
    }

    wchar_t* escaped = (wchar_t*)malloc((length + (space_count * 2) + 1) * sizeof(*escaped));
    if (!escaped) {
        return NULL;
    }

    wchar_t* destination = escaped;
    for (const wchar_t* source = url; *source; source++) {
        if (*source == L' ') {
            *destination++ = L'%';
            *destination++ = L'2';
            *destination++ = L'0';
        } else {
            *destination++ = *source;
        }
    }
    *destination = L'\0';

    return escaped;
}

// Check if URL is valid
BOOL is_valid_url(const wchar_t* url) {
    return url && wcslen(url) > 0 && (wcsstr(url, L"http://") == url || wcsstr(url, L"https://") == url);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(nCmdShow);

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

    const wchar_t* url_start = pCmdLine;
    if (wcsncmp(url_start, L"mpc-hc://", 9) == 0) {
        url_start += 9;
        log_message(L"Removed mpc-hc:// prefix: %ls", url_start);
    }

    wchar_t* url = _wcsdup(url_start);
    if (!url) {
        log_message(L"Error: Memory allocation failed");
        return 1;
    }
    wchar_t* final_url = NULL;

    // 1. Parse weblink format (detect weblink?url= or weblink/?url=)
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
        // 2. Handle standard format and Chrome 130+ broken format
        final_url = fix_broken_url(url);
    }

    if (!final_url) {
        return 1;
    }

    // Validate URL
    if (!is_valid_url(final_url)) {
        log_message(L"Error: Invalid URL: %ls", final_url);
        free(final_url);
        return 1;
    }

    wchar_t* escaped_url = escape_url_spaces(final_url);
    free(final_url);
    final_url = escaped_url;
    if (!final_url) {
        log_message(L"Error: Failed to escape URL");
        return 1;
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
    INT_PTR ret = (INT_PTR)ShellExecuteW(NULL, NULL, mpc_path, args, NULL, SW_SHOWNORMAL);
    if (ret <= 32) {
        log_message(L"Error: Failed to start MPC-HC (error code: %lld)", (long long)ret);
        free(args);
        free(final_url);
        return (int)ret;
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
