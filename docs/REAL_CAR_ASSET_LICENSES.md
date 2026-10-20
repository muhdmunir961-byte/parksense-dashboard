# ParkSense real-car reference provenance

ParkSense now presents **licensed photographs** for certain Malaysian-car generations, and separately supports **creator-hosted 3D embeds** when available.

## Photo references (Wikimedia Commons)

| Dashboard profile | Photo file | Photographer / credit | License |
|---|---|---|---|
| Perodua Myvi Gen 1 | [Perodua Myvi EZi original look 2006.jpg](https://commons.wikimedia.org/wiki/File:Perodua_Myvi_EZi_original_look_2006.jpg) | Saifol | Attribution required |
| Myvi Gen 1 facelift | Same original Gen 1 reference (not facelift-specific) | Saifol | Attribution required |
| Myvi Gen 2 | [Perodua Myvi (1).jpg](https://commons.wikimedia.org/wiki/File:Perodua_Myvi_(1).jpg) | Bindydad123 | CC BY-SA 4.0 |
| Myvi Gen 2 facelift | Same pre-facelift Gen 2 reference (not facelift-specific) | Bindydad123 | CC BY-SA 4.0 |
| Myvi Gen 3 | [2018 Myvi Advance.jpg](https://commons.wikimedia.org/wiki/File:2018_Myvi_Advance.jpg) | Pdudkronos | CC BY-SA 4.0 |
| Myvi Gen 3 facelift | [2021 Perodua Myvi 1.5 AV (Malaysia) front view.png](https://commons.wikimedia.org/wiki/File:2021_Perodua_Myvi_1.5_AV_(Malaysia)_front_view.png) | Automachi | CC BY 3.0 |
| Proton Saga Gen 1 | [Proton Saga 1985.jpg](https://commons.wikimedia.org/wiki/File:Proton_Saga_1985.jpg) | Liauzh | CC BY-SA 4.0 |

Photos are loaded from Wikimedia Commons only on matching selections and credited with an in-app link to the original file/licensing page. No photo is used as an AI training input. Wikimedia uploads can be replaced, removed, or rate-limited by their host.

## Creator-hosted 3D embeds (models not downloaded)

| Model option | Provider | Creator | Conditions |
|---|---|---|---|
| Myvi (all selections) | [Sketchfab – Perodua MYVI Stock And Mod](https://sketchfab.com/3d-models/perodua-myvi-stock-and-mod-4c8a2edd636f478994f021b9279ea1e1) | un | NoAI; **view/embed only**. Exact Myvi generation unverified. Not copied, downloaded, adapted or passed to an AI model. |
| Saga Gen 1 (1985) | [Sketchfab – Proton Saga car red 1985](https://sketchfab.com/3d-models/proton-saga-car-red-1985-7e8adb5d0d2e4dfca405e07567835406) | zairiq-123 | CC BY; creator credit and source link displayed. Public embedded preview. |

A third-party viewer is instantiated **only after user taps CREATOR 3D** and is unloaded on exit. Internet is required, and Sketchfab availability/device support is outside ParkSense's control.

## Do not overstate the results

- The original ParkSense `3D 360°` view remains a lightweight **procedural geometry illustration** and is **not** photogrammetry, OEM CAD, or a photorealistic mesh.
- REAL PHOTO is a **2D photograph**, not a texture that can produce true unseen 360-degree views.
- CREATOR 3D is a separate, correctly sourced 3D model from its original author. It is **not** generated, rebuilt or owned by ParkSense.
- The body style/generation selections do not alter sensor readings, ESP-NOW communication, GPS measurements, physical buzzer logic, or sensor calibration.
- To get a truly faithful Myvi Gen 1/2/3 360° model, create/obtain original model-specific orthographic front/side/rear/roof reference images, use a permitted Blender modeling workflow or approved photogrammetry from overlapping angles, and export optimized `.glb` meshes with documented rights. Each generation requires its own assets.

This feature updates only the dashboard and its web security policy; ESP32 firmware is unchanged.
