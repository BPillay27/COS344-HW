#!/bin/bash
# Run the HomwWorkAssignment executable from the correct directory

cd "$(dirname "$0")"
./out/build/x64-debug/HomwWorkAssignment.exe
