#!/usr/bin/env python3
"""
Regenerates CMAKE/SOURCES.cmake and CMAKE/TESTS.cmake by scanning directories.

This script automatically detects SOURCE and TESTS directories in the project
and any submodules, then generates CMake variable files with all .h/.cpp files.

Run this whenever you add new source or test files to automatically update
the CMake file lists.
"""

import os
from pathlib import Path


def generate_files_list(root_folders, output_file, variable_name):
    """
    Generate a CMake set() command listing all .h/.cpp files in given folders.

    Args:
        root_folders: List of directories to scan
        output_file: Path to output CMake file
        variable_name: Name of the CMake variable
    """
    files_list = []
    for root_folder in root_folders:
        if not os.path.exists(root_folder):
            continue
        for folder, subfolders, files in os.walk(root_folder):
            for filename in files:
                if filename.endswith('.cpp') or filename.endswith('.h'):
                    # Use forward slashes for CMake cross-platform compatibility
                    file_path = os.path.join(folder, filename).replace('\\', '/')
                    files_list.append(file_path)

    files_list.sort()

    if files_list:
        files_list_str = "\n    ".join(files_list)
        output = 'set({}\n    {}\n)'.format(variable_name, files_list_str)
    else:
        # Empty list
        output = 'set({}\n    # No files found\n)'.format(variable_name)

    with open(output_file, 'w+') as f:
        f.write(output)
    print(f"Generated {output_file} with {len(files_list)} file(s)")


def discover_source_folders():
    """
    Discover all SOURCE directories in project and submodules.

    Returns:
        List of directory paths
    """
    folders = []

    # Main SOURCE directory
    if os.path.exists('SOURCE'):
        folders.append('SOURCE')

    # Check for SOURCE in any submodules (excluding JUCE)
    submodules_dir = Path('SUBMODULES')
    if submodules_dir.exists():
        for item in submodules_dir.iterdir():
            if item.is_dir() and item.name != 'JUCE':
                source_dir = item / 'SOURCE'
                if source_dir.exists():
                    folders.append(str(source_dir).replace('\\', '/'))

    return folders


def discover_test_folders():
    """
    Discover TESTS directories in the main project and submodules.

    Returns:
        List of directory paths
    """
    folders = []

    # Main TESTS directory
    if os.path.exists('TESTS'):
        folders.append('TESTS')

    return folders


def main():
    """Main entry point."""
    # Discover and generate source files
    source_folders = discover_source_folders()
    if source_folders:
        print(f"Scanning source folders: {', '.join(source_folders)}")
    else:
        print("Warning: No SOURCE folders found")
    generate_files_list(source_folders, 'CMAKE/SOURCES.cmake', 'SOURCES')

    # Discover and generate test files
    test_folders = discover_test_folders()
    if test_folders:
        print(f"Scanning test folders: {', '.join(test_folders)}")
    else:
        print("Warning: No TESTS folders found")
    generate_files_list(test_folders, 'CMAKE/TESTS.cmake', 'TEST_SOURCES')


if __name__ == '__main__':
    main()
