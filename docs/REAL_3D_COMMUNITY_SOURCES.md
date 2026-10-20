# ParkSense community 3D model sources

The website uses externally hosted models with the creators' own interactive Sketchfab viewer. These are original community-authored 3D meshes, not AI-reconstructed photos or manufacturer CAD.

## Verified models

1. **Perodua Myvi** by **zairiq-123** — CC Attribution (CC BY).
   Source: https://sketchfab.com/3d-models/myvi-4f62bfca654643c3858a7688e7a708ed
   Sketchfab ID: `4f62bfca654643c3858a7688e7a708ed`.
   The creator did not identify generation: **do not claim an exact match to Gen1/2/3**.

2. **Proton Saga car red 1985** by **zairiq-123** — CC Attribution (CC BY).
   Source: https://sketchfab.com/3d-models/proton-saga-car-red-1985-7e8adb5d0d2e4dfca405e07567835406
   Sketchfab ID: `7e8adb5d0d2e4dfca405e07567835406`.
   Model is community authored (not OEM CAD) and may be large on older tablets.

Dashboard file `public/index.html` contains the `reference3DModels` mapping with author credits and source links, `REAL 3D ↗` toggle, lazy-loading Sketchfab iframe only on user selection, and clear warnings for generation mismatch. It preserves 2D TOP, on-device geometric 3D 360, optional licensed photo references and read-only ESP32 telemetry. CSP in `server.js` allows `frame-src https://sketchfab.com`. Embedding is subject to Sketchfab terms; no third-party 3D binary is copied into this repo. License/attribution links remain visible in the UI.

## Accurate generation-specific models: future work

To build faithful Myvi Gen1/2/3, Saga variants, etc., collect licensed multi-angle reference photos (front, back, side and top), then create/verify a Blender mesh and optimize it to local .glb (ideally under 1–3 MB per car). Render with a web glTF viewer with touch orbit. Verify copyright, permission and proper CC BY credit before distributing derived assets. Do not extract models not offered for download, or misrepresent a generic mesh as OEM-accurate.

Sketchfab documentation: https://sketchfab.com/developers/viewer/initialization
License primer: https://sketchfab.com/blogs/community/an-introduction-to-creative-commons-licenses/
