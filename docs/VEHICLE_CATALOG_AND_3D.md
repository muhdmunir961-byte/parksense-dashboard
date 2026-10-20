# ParkSense Malaysia Vehicle Catalog & 3D Viewer

## Included in dashboard
- 23 brands, 145 named models and 211 selectable model/generation labels.
- All selections are UI-only and are never sent to ESP32.
- The first 13 generation/profile IDs contain hand-authored approximate body geometry (Myvi Gen 1/2/3/facelift, Axia Gen 1/2, Bezza, Saga Gen 3/new generation, Persona, X50, City, Vios).
- Other models intentionally use one of six generic body templates: hatchback, sedan, SUV, MPV, pickup or coupe.
- 2D top-view car illustration is inline SVG. The 3D 360-degree model is a canvas-rendered procedural polygon mesh; it is not a manufacturer's 3D CAD model.
- Tablet supports one-finger drag orbit (also mouse drag); desktop mouse wheel and keyboard +/- allow zoom; arrow keys rotate/change pitch. No outside model files are loaded.
- User selection is stored in browser localStorage (brand, model, generation and color). The catalog does not require external services.

## Model quality and licensing
- Brand/model text names are catalogue references and do not imply endorsement by manufacturers.
- Generated 3D meshes are conceptual silhouettes without manufacturer CAD, trim specifications or photoreal accuracy.
- The selected profile DOES NOT change sensor mounting position, sensor readings, sensor calibration, measured proximity thresholds or vehicle driving dynamics.
- A future replacement with accurate externally sourced GLB/glTF models requires rights clearance and proper attribution according to each asset license.
- Kenney Car Kit (generic vehicles, CC0): https://kenney-assets.itch.io/car-kit
- Perodua current model list: https://www.perodua.com.my/our-models/choose-model
- Proton owners manuals and models: https://www.proton.com/after-sales/owners-manual
- Honda Malaysia: https://www.honda.com.my/
- Toyota Malaysia: https://www.toyota.com.my/en/price-and-model-tools/model-brochures.html
- Myvi generation timeline: https://paultan.org/2017/11/16/perodua-myvi-through-the-years-from-2005-to-2017/
- Myvi 2021 update: https://www.perodua.com.my/articles/new-myvi-launched-with-4303-units-booked-within-9-days

## How to try
1. Deploy latest GitHub branch via Coolify.
2. Sign in at your dashboard, then open Overview.
3. Choose Brand → Model → Generation and paint color.
4. Use 2D TOP or 3D 360° toggle. Drag the car to rotate.
5. Return to Parking Assist; the 2D silhouette matches the selected model.
6. Demo scenarios and live telemetry stay intact.

## Next steps for high-fidelity models
1. Commission original Blender low-poly assets for the most-used vehicles, one separately identifiable generation at a time.
2. Verify licensing and make per-generation optimized GLB/glTF files.
3. Add a local 3D viewer and static file allowlist to serve GLBs. Support fallback generic geometry on slow devices.
4. Keep hazard detection independent of the vehicle rendering.

## Known limitations
- The catalogue is a broad starter list, not an exhaustive or authoritative list of every Malaysian-market make, subvariant or production year.
- Generation and production year labels should be verified against individual manufacturer records before any publication requiring historical accuracy.
- The 3D renderer and selections have passed JavaScript syntax and static checks but still require an actual browser/tablet visual test after deployment.
