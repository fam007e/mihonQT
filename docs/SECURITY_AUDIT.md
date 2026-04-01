# Security Audit: MihonQT

This document outlines the security considerations and findings for the MihonQT project.

## 1. Database Security (SQL Injection)
- **Status:** ✅ Secure
- **Findings:** All database interactions in `src/database/` use `QSqlQuery::prepare` and `QSqlQuery::bindValue`. This ensures that user-supplied data is correctly escaped and parameterized, preventing SQL injection vulnerabilities.

## 2. Extension Sandboxing (JavaScript)
- **Status:** ✅ Secure (Restricted)
- **Findings:** MihonQT uses `QJSEngine` with a custom trust-based sandboxing system.
    - **Trust Mechanism:** Extensions are untrusted by default.
    - **Network Isolation:** Untrusted extensions are restricted to making network requests only to their respective base URLs (e.g., `mangadex.org`).
    - **User Control:** Users can explicitly grant "Trust" to extensions in the Extension Manager, which removes these restrictions.
- **Mitigation:** Implemented in April 2026.

## 3. Network Security
- **Status:** ✅ Secure
- **Findings:** A global security policy for network requests has been implemented.
    - **Enforce HTTPS:** A new toggle in Settings > Security allows users to block all non-HTTPS traffic from extensions.
    - **Policy Enforcement:** All network requests from the JavaScript engine are intercepted and validated by the `NetworkAccessManager` against the user's security preferences.
- **Mitigation:** Implemented in April 2026.

## 4. Data Privacy
- **Status:** ✅ Secure
- **Findings:** 
    - **Incognito Mode:** Pauses both history recording and local chapter progress updates.
    - **Local Storage:** Data is stored in the standard system data locations (e.g., `~/.local/share/MihonQT`).
- **Considerations:** Sensitive user credentials (like tracker tokens) are planned to be moved to the system keychain for future enhanced security.

## 5. Memory Safety (C++)
- **Status:** ✅ Improved
- **Findings:** The project has undergone a memory safety refactoring.
    - **Smart Pointers:** Core components, including the `DatabaseManager` and its repositories, have been transitioned to use `std::unique_ptr` and `std::shared_ptr`.
    - **Ownership Model:** Leverages Qt's parent-child ownership for UI components while using modern C++ for core resource management.
- **Mitigation:** Implemented in April 2026.
