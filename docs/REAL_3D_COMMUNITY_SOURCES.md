# ParkSense — Real-3D-only vehicle selector (Option B)

## Shown on Vehicle Overview

Exactly **two selectable cars** are shown. Other brands, model names, generations and procedural-3D view modes are no longer exposed on Vehicle Overview.

1. **Perodua Myvi** — model by **zairiq-123**, CC Attribution / CC BY. Generation **unverified**; never imply this is a faithful Myvi Gen 1, 2 or 3 mesh.
   - Original: https://sketchfab.com/3d-models/myvi-4f62bfca654643c3858a7688e7a708ed
   - Sketchfab ID: `4f62bfca654643c3858a7688e7a708ed`
   - 36.8k triangles / 22.7k vertices as listed by creator.

2. **Proton Saga 1985** — model by **zairiq-123**, CC Attribution / CC BY. Original first-generation Saga interpretation, not manufacturer CAD.
   - Original: https://sketchfab.com/3d-models/proton-saga-car-red-1985-7e8adb5d0d2e4dfca405e07567835406
   - Sketchfab ID: `7e8adb5d0d2e4dfca405e07567835406`
   - 340.2k triangles / 200.3k vertices, potentially slow on older tablets.

## Implementation

- Main file: `public/index.html`. Brand/model chooser replaced with two selectable buttons, with exactly one internal generation label per asset.
- Previously stored localStorage choices for unavailable vehicles fall back safely to the available Myvi or Saga 1985 model. The original two catalog dropdowns remain hidden only for backward compatibility.
- Creator-hosted real Sketchfab 3D viewer is displayed **automatically** on opening Overview and changes with model selection. This loads content from Sketchfab; internet access is required.
- 2D and procedurally generated 3D are **not offered as model viewer options**. The Parking Assist page retains a separate top-view **schematic** illustrating the four physical ESP-NOW sensor positions, not a claim of official car geometry.
- Creator links and generation-accuracy caveats appear directly below each viewer.
- No Sketchfab 3D binary is copied, altered, re-licensed or rehosted. The owner retains the original model.
- ESP32 firmware, GPS, sensor thresholds, browser warning events, physical LED/buzzer behavior and server authorization are untouched.
- Next real 3D model should be added only when artist identity, license, model identity and generation match have been checked.

## QA and known limitations

JavaScript syntax and duplicate DOM IDs are checked prior to GitHub push. Coolify deployment and real-tablet rendering must still be verified after deployment. Third-party model availability can change.
