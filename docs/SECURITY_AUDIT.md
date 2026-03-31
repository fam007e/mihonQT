# Security Audit: MihonQT

This document outlines the security considerations and findings for the MihonQT project.

## 1. Database Security (SQL Injection)
- **Status:** ✅ Secure
- **Findings:** All database interactions in `src/database/` use `QSqlQuery::prepare` and `QSqlQuery::bindValue`. This ensures that user-supplied data is correctly escaped and parameterized, preventing SQL injection vulnerabilities.

## 2. Extension Sandboxing (JavaScript)
- **Status:** ⚠️ Partial Risk
- **Findings:** MihonQT uses `QJSEngine` to run source extensions. While `QJSEngine` provides a restricted environment, the application explicitly exposes the following powerful C++ objects:
    - `Network`: `NetworkAccessManager` (Allows network requests).
    - `Html`: `HtmlParser` (Allows HTML parsing).
- **Risks:** A malicious extension could potentially:
    - Scan the local network using the `Network` object.
    - Attempt to exploit vulnerabilities in the HTML parser or network stack.
- **Recommendation:** Implement a permission-based system or a "Trust" mechanism for extensions (similar to the Android version).

## 3. Network Security
- **Status:** ⚠️ Informational
- **Findings:** The application relies on extensions to provide URLs. There is currently no global policy enforcing HTTPS.
- **Risks:** Extensions using plain HTTP are vulnerable to Man-in-the-Middle (MITM) attacks.
- **Recommendation:** Add an option to "Enforce HTTPS" in settings and warn users when an extension uses non-secure protocols.

## 4. Data Privacy
- **Status:** ✅ Secure
- **Findings:** 
    - **Incognito Mode:** Pauses both history recording and local chapter progress updates.
    - **Local Storage:** Data is stored in the standard system data locations (e.g., `~/.local/share/MihonQT`).
- **Considerations:** No sensitive user credentials (like passwords for tracking services) are currently stored. If added in the future, they should use the system keychain (Secret Service API on Linux, Keychain on macOS, Credential Manager on Windows).

## 5. Memory Safety (C++)
- **Status:** ⚠️ General C++ Risks
- **Findings:** The project uses standard C++ and Qt patterns. 
- **Recommendation:** Use `std::unique_ptr` and `std::shared_ptr` where possible instead of raw pointers to minimize memory leaks and use-after-free vulnerabilities. Continue using Qt's parent-child ownership model.
