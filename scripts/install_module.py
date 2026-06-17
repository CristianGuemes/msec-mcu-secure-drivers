"""
Script to install the msec-mcu-secure-drivers module into the
Harmony Content folder structure.

Actions performed:
  1. Copy project to <harmony_content_path>/harmony/content/secure_drivers/<version>
  2. Copy and update mcc-index.json with the correct version
  3. Copy secure.yml to the catalog directory
  4. Add "Microchip Secure Services" link to catalog.yml (if not present)

Where <version> is read from package.yml (field package.version).

If no harmony_content_path is provided, the script auto-detects it from
the MPLAB IDE mcc.properties file (MCC_ROOT_PATH variable), using the
most recent MPLAB version found.

Usage:
    python install_module.py [harmony_content_path]

Examples:
    python install_module.py C:\\Users\\user\\MicrochipHarmony
    python install_module.py
    -> auto-detects from mcc.properties
"""

import argparse
import json
import os
import re
import shutil
import sys
from pathlib import Path

import yaml

# Project root is one level up from this script's location
SCRIPTS_DIR = Path(__file__).resolve().parent
PROJECT_ROOT = SCRIPTS_DIR.parent
PACKAGE_YML = PROJECT_ROOT / "package.yml"
MCC_INDEX_JSON = SCRIPTS_DIR / "mcc-index.json"
SECURE_YML = SCRIPTS_DIR / "secure_services.yml"

# Catalog directory name inside harmony/catalog/
CATALOG_DIR_NAME = "https--github.com-Microchip-MPLAB-Harmony-catalog.git"
CATALOG_LINK_ENTRY = '        - { name: "Microchip Secure Services", url: "file:///secure_services.yml"}'

# Patterns to exclude from the copy
EXCLUDE_DIRS = {".git", "scripts"}
EXCLUDE_FILES = {".gitignore"}


def ignore_patterns(directory, contents):
    """Return set of names to ignore during copy."""
    ignored = set()
    for item in contents:
        if item in EXCLUDE_DIRS or item in EXCLUDE_FILES:
            ignored.add(item)
    return ignored


def get_mcc_root_path() -> str:
    """Auto-detect MCC_ROOT_PATH from mcc.properties in the most recent MPLAB version."""
    user_profile = os.environ.get("USERPROFILE")
    if not user_profile:
        print("Error: USERPROFILE environment variable not found.")
        sys.exit(1)

    mplab_dev_dir = Path(user_profile) / "AppData" / "Roaming" / "mplab_ide" / "dev"

    if not mplab_dev_dir.exists():
        print(f"Error: MPLAB IDE config directory not found: {mplab_dev_dir}")
        sys.exit(1)

    # Find all version directories (vX.XX format) and pick the most recent
    version_dirs = []
    for entry in mplab_dev_dir.iterdir():
        if entry.is_dir() and re.match(r"^v\d+\.\d+$", entry.name):
            version_dirs.append(entry)

    if not version_dirs:
        print(f"Error: No MPLAB version directories found in {mplab_dev_dir}")
        sys.exit(1)

    # Sort by version number (major, minor) descending
    def version_key(p):
        match = re.match(r"^v(\d+)\.(\d+)$", p.name)
        return (int(match.group(1)), int(match.group(2)))

    version_dirs.sort(key=version_key, reverse=True)
    latest_version_dir = version_dirs[0]

    mcc_properties = (
        latest_version_dir
        / "config"
        / "Preferences"
        / "com"
        / "microchip"
        / "mcc.properties"
    )

    if not mcc_properties.exists():
        print(f"Error: mcc.properties not found at {mcc_properties}")
        sys.exit(1)

    # Read MCC_ROOT_PATH from the properties file
    with open(mcc_properties, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line.startswith("MCC_ROOT_PATH="):
                value = line.split("=", 1)[1]
                # Properties files may use escaped backslashes
                value = value.replace("\\\\", "\\")
                print(f"Auto-detected MCC_ROOT_PATH from MPLAB {latest_version_dir.name}: {value}")
                return value

    print(f"Error: MCC_ROOT_PATH not found in {mcc_properties}")
    sys.exit(1)


def get_version() -> str:
    """Read the version from package.yml."""
    if not PACKAGE_YML.exists():
        print(f"Error: package.yml not found at {PACKAGE_YML}")
        sys.exit(1)

    with open(PACKAGE_YML, "r", encoding="utf-8") as f:
        data = yaml.safe_load(f)

    try:
        version = data["package"]["version"]
    except (KeyError, TypeError):
        print("Error: Could not read 'package.version' from package.yml")
        sys.exit(1)

    if not version:
        print("Error: 'package.version' is empty in package.yml")
        sys.exit(1)

    return version


def install_catalog(harmony_root: Path) -> None:
    """Copy secure.yml to catalog dir and add link entry to catalog.yml if missing."""
    catalog_dir = harmony_root / "harmony" / "catalog" / CATALOG_DIR_NAME

    if not catalog_dir.exists():
        print(f"Error: Catalog directory not found: {catalog_dir}")
        sys.exit(1)

    # Copy secure.yml to catalog directory
    if not SECURE_YML.exists():
        print(f"Error: secure.yml not found at {SECURE_YML}")
        sys.exit(1)

    dest_secure_yml = catalog_dir / "secure_services.yml"
    shutil.copy2(SECURE_YML, dest_secure_yml)
    print(f"secure_services.yml copied to {catalog_dir}")

    # Add link entry to catalog.yml if not already present
    catalog_yml = catalog_dir / "catalog.yml"
    if not catalog_yml.exists():
        print(f"Error: catalog.yml not found at {catalog_yml}")
        sys.exit(1)

    with open(catalog_yml, "r", encoding="utf-8") as f:
        content = f.read()

    link_check = '"Microchip Secure Services"'
    if link_check in content:
        print("catalog.yml already contains the Secure Services link, skipping.")
    else:
        # Find the last link entry and append after it
        lines = content.splitlines(keepends=True)
        insert_index = None
        for i, line in enumerate(lines):
            if "url:" in line and "file:///" in line and "links:" not in line:
                insert_index = i

        if insert_index is None:
            print("Error: Could not find links section in catalog.yml")
            sys.exit(1)

        lines.insert(insert_index + 1, CATALOG_LINK_ENTRY + "\n")

        with open(catalog_yml, "w", encoding="utf-8") as f:
            f.writelines(lines)

        print("Added 'Microchip Secure Services' link to catalog.yml")


def copy_project(harmony_content_path: str) -> None:
    harmony_root = Path(harmony_content_path).resolve()

    if not harmony_root.exists():
        print(f"Error: Harmony Content path does not exist: {harmony_root}")
        sys.exit(1)

    version = get_version()
    dest = harmony_root / "harmony" / "content" / "secure_services" / version

    print(f"Source:      {PROJECT_ROOT}")
    print(f"Version:     {version}")
    print(f"Destination: {dest}")
    print("Copying...")

    shutil.copytree(PROJECT_ROOT, dest, ignore=ignore_patterns, dirs_exist_ok=True)

    # Copy and update mcc-index.json with the correct version
    if not MCC_INDEX_JSON.exists():
        print(f"Warning: mcc-index.json not found at {MCC_INDEX_JSON}, skipping.")
    else:
        with open(MCC_INDEX_JSON, "r", encoding="utf-8") as f:
            index_data = json.load(f)

        index_data["body"]["version"] = version

        dest_index = dest / "mcc-index.json"
        with open(dest_index, "w", encoding="utf-8") as f:
            json.dump(index_data, f)

        print(f"mcc-index.json copied and updated with version {version}")

    # Install catalog entries
    install_catalog(harmony_root)

    print("Done.")


def main():
    parser = argparse.ArgumentParser(
        description="Copy msec-mcu-secure-drivers project into Harmony Content folder."
    )
    parser.add_argument(
        "harmony_content_path",
        nargs="?",
        default=None,
        help="Root path of the Harmony Content installation. "
        "If not provided, auto-detects from MPLAB mcc.properties.",
    )
    args = parser.parse_args()

    if args.harmony_content_path:
        harmony_path = args.harmony_content_path
    else:
        harmony_path = get_mcc_root_path()

    copy_project(harmony_path)


if __name__ == "__main__":
    main()
