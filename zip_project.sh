#!/bin/bash
# ==============================================================================
# Project Zip Script for Skyline Sprint
# Pack clean source files (respects .gitignore, excludes build artifacts/patches)
# ==============================================================================
set -e

OUTPUT_ZIP="skyline_sprint.zip"
if [ -n "$1" ]; then
    OUTPUT_ZIP="$1"
fi

echo "Creating clean project ZIP: $OUTPUT_ZIP..."

# Clean up existing zip if it exists to prevent appending
rm -f "$OUTPUT_ZIP"

# Get all files tracked by git + untracked non-ignored files,
# filter out WSL Zone.Identifier metadata, temporary patch files, and the output zip itself.
git ls-files --cached --others --exclude-standard | \
  grep -v ":Zone.Identifier" | \
  grep -v "\.patch$" | \
  grep -v "$OUTPUT_ZIP" | \
  grep -v "zip_project.sh" | \
  zip -q "$OUTPUT_ZIP" -@

echo "ZIP archive created successfully containing clean repository state!"
echo "Archived files summary:"
unzip -l "$OUTPUT_ZIP" | tail -n 10
