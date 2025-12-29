# CppCheck Configuration

Add to your workflow or local runs:
```bash
cppcheck --enable=all --suppressions-list=.cppcheck src/
```

The `.cppcheck` file suppresses Qt-specific warnings and style suggestions.

**Note**: CppCheck doesn't auto-load suppression files. You must use the `--suppressions-list` flag.
