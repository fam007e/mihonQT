# Security Policy

## Supported Versions

The following versions of MihonQT are currently being supported with security updates.

| Version | Supported          |
| ------- | ------------------ |
| latest  | :white_check_mark: |

## Reporting a Vulnerability

If you discover a security vulnerability within this project, please send an e-mail to the maintainer at [email](mailto:faisalmoshiur+mihonqt@gmail.com). All security vulnerabilities will be promptly addressed.

Please include:
- The version of the project you are using.
- A description of the vulnerability.
- Steps to reproduce the vulnerability.

We appreciate your help in keeping MihonQT secure.

## Third-Party Dependencies

This project uses third-party libraries (located in `libs/`) that are maintained by their respective upstream projects. Security scanning (e.g., Flawfinder, CodeQL) is configured to exclude these directories, as:

1. These libraries have their own security processes and maintainers
2. We track security advisories for dependencies through GitHub's Dependabot
3. We regularly update submodules to incorporate upstream security fixes

If you discover a vulnerability in a third-party dependency, please report it to the upstream project maintainers directly.
