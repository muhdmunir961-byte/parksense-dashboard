#!/usr/bin/env python3
"""Generate the one untracked ESP32 cloud config (no external Python dependencies).

Never sends passwords or DEVICE_TOKEN to GitHub. Optional PlatformIO upload requires
an explicit local acknowledgement and a connected, user-verified board.
"""
from __future__ import annotations
import argparse
import getpass
import json
import pathlib
import shutil
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
FIRMWARE = ROOT / "firmware" / "esp32-main"
OUT = FIRMWARE / "include" / "cloud_config.h"
ENDPOINT = "https://parkingdashboard.my.id/api/device/telemetry"


def text(prompt: str, secret: bool = False) -> str:
    return (getpass.getpass(prompt) if secret else input(prompt)).strip()


def cpp(value: str) -> str:
    """Escape values as C++ UTF-8-compatible narrow string literals."""
    return json.dumps(value, ensure_ascii=False)


def write_config() -> None:
    print("\nParkSense — ONE-TIME ESP32 SETUP")
    print("No passwords or tokens will be sent anywhere by this script.")
    print("WARNING: ESP-NOW pods are fixed to 2.4GHz CH1. Verify Android hotspot CH1 before flashing.\n")
    verified = text("Is your hotspot confirmed 2.4 GHz / CHANNEL 1? Type CH1: ")
    if verified != "CH1":
        sys.exit("Stopped safely. Verify channel 1 using a Wi-Fi analyzer first; no file written.")

    ssid = text("Android hotspot Wi-Fi name (SSID): ")
    password = text("Android hotspot password (input hidden): ", secret=True)
    token = text("DEVICE_TOKEN from Coolify (input hidden, 24+ chars): ", secret=True)
    ap_password = text("ESP32 local AP password (input hidden, 8-63 chars): ", secret=True)

    errors = []
    if not 1 <= len(ssid.encode("utf-8")) <= 32:
        errors.append("SSID must be 1-32 UTF-8 bytes.")
    if not 8 <= len(password.encode("utf-8")) <= 63:
        errors.append("Hotspot password must be 8-63 bytes.")
    if len(token) < 24 or token == password or token == ap_password:
        errors.append("Coolify DEVICE_TOKEN must be at least 24 characters and different from Wi-Fi passwords.")
    if not 8 <= len(ap_password.encode("utf-8")) <= 63:
        errors.append("ESP32 local AP password must be 8-63 bytes.")
    if errors:
        sys.exit("Stopped: " + " ".join(errors))

    content = (
        "#pragma once\n"
        "// GENERATED LOCALLY; KEEP PRIVATE. .gitignore excludes this file.\n"
        "#define CLOUD_UPLINK_ENABLED true\n"
        f"#define CLOUD_WIFI_SSID {cpp(ssid)}\n"
        f"#define CLOUD_WIFI_PASSWORD {cpp(password)}\n"
        f"#define CLOUD_API_URL {cpp(ENDPOINT)}\n"
        f"#define CLOUD_DEVICE_TOKEN {cpp(token)}\n"
        f"#define PARKSENSE_AP_PASSWORD {cpp(ap_password)}\n"
    )
    if OUT.exists():
        if text("cloud_config.h already exists. Overwrite it? Type YES: ") != "YES":
            sys.exit("Existing config preserved.")
    OUT.parent.mkdir(parents=True, exist_ok=True)
    OUT.write_text(content, encoding="utf-8")
    try:
        OUT.chmod(0o600)
    except OSError:
        pass
    print("\nCreated local private configuration in firmware/esp32-main/include/cloud_config.h")
    print("Public dashboard:", "https://parkingdashboard.my.id")
    print("Keep ESP32 on the SAME CH1 hotspot and connect the tablet to internet.")
    print("DO NOT push this config file to GitHub.")


def flash(port: str | None) -> None:
    cmd = ["pio", "run", "-d", str(FIRMWARE), "-t", "upload"]
    if port:
        cmd += ["--upload-port", port]
    if not shutil.which("pio"):
        sys.exit("PlatformIO CLI (pio) not found. Install PlatformIO in VS Code / code-server first.")
    print("\nReady to compile and upload main ESP32 (sensor pods and physical alarms are unchanged).")
    if text("Confirm board + wiring and type FLASH to start: ") != "FLASH":
        sys.exit("Upload cancelled. No firmware flashed.")
    subprocess.run(cmd, cwd=str(ROOT), check=True)


def main() -> None:
    parser = argparse.ArgumentParser(description="Prepare ParkSense main ESP32 for one-click cloud upload")
    parser.add_argument("--upload", action="store_true", help="Ask before compiling and uploading the main ESP32")
    parser.add_argument("--port", help="Optional explicit serial port such as COM5 or /dev/ttyUSB0")
    args = parser.parse_args()
    try:
        write_config()
        if args.upload:
            flash(args.port)
        else:
            print("NEXT: Plug in the main ESP32 and run: pio run -d firmware/esp32-main -t upload")
            print("Or re-run with --upload (interactive confirmation).")
        print("Open Serial Monitor at 115200; verify pods F/R/L/Right and cloud status.")
    except (KeyboardInterrupt, EOFError):
        sys.exit("\nCancelled without flashing.")
    except subprocess.CalledProcessError as exc:
        sys.exit(f"PlatformIO failed with code {exc.returncode}; check serial port, libraries and compiler output.")


if __name__ == "__main__":
    main()
