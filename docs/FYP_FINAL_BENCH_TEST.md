# ParkSense — final FYP bench test sheet

Use this sheet after flashing the **main ESP32**. Do not test the prototype by driving in live traffic. Use a safe stationary test rig; keep original firmware on hand.

## Preflight — MUST ALL PASS
- [ ] Android hotspot or dedicated 2.4 GHz router is verified on **channel 1** (2412 MHz).
- [ ] Local AP **Monitor Dashboard** appears and you can still open `http://192.168.4.1` as fallback.
- [ ] Main ESP32 boot output shows ESP-NOW initialization success on serial **115200 baud**.
- [ ] All four independent pods (front, rear, left, right) send changing distances on channel 1; OFFLINE pod produces `-1`.
- [ ] Only the **main** ESP32 was flashed during setup; pod firmware was not overwritten without a justified change.

## Physical safety (with GPIO14 enabled; verify wiring)
- [ ] No obstacle, no PIR movement, no overspeed: GREEN indicator if an online pod exists; no buzzer.
- [ ] **35–54 cm**: YELLOW + slow warning beeps.
- [ ] **15–34 cm**: RED + intermittent fast beeps.
- [ ] **Under 15 cm**: solid RED + continuous warning beep.
- [ ] Trigger PIR and confirm the correct distinctive warning sequence.
- [ ] Encoder changes speed-alert threshold (5–100 km/h); push returns threshold to 15.
- [ ] GPIO14 switch OFF disables physical alert outputs.
- [ ] Unplug one pod: dashboard marks only that pod OFFLINE and hardware should not claim a false clear state.
- [ ] GPS signal loss: recent-fix missing should be represented as no reliable speed; reported speed zero is **not** proof of physical stopped vehicle.

## Cloud & tablet
- [ ] Login to `https://parkingdashboard.my.id` with correct dashboard password.
- [ ] Website is online; Settings self-check shows **WEBSITE SERVER ONLINE**.
- [ ] Main ESP32 successfully posts telemetry (independent of WebSocket).
- [ ] Settings shows **ESP32 DEVICE RECEIVING DATA** when packets arrive.
- [ ] Tablet Dashboard updates distances, PIR, speed, limit and parking switch correctly.
- [ ] Stop ESP32 uploads; within ~5 seconds dashboard indicates device offline rather than freezing at apparently live numbers.
- [ ] Enable browser warning audio once and test normal, caution, danger and overspeed. Acknowledge affects **browser only**, never LED/buzzer.
- [ ] Opening **Myvi** and **Saga 1985** loads real, attributed CC BY 3D models with user-controlled orbit. Do not say Myvi's generation is confirmed.
- [ ] Cloud Wi-Fi fails: physical alarm logic and local AP still work (do not rely on Cloud for warning).

## Important FAIL conditions
- **Hotspot not on CH1:** STOP cloud pairing; radio coexistence not verified.
- **TLS/HTTPS errors:** validate actual domain certificate CA chain; never use `setInsecure()`.
- **Wrong Wi-Fi, 401 or HTTP errors:** inspect local config and Coolify variables; do not post secrets to chats/logs.
- **Pod offline or buzzer not operating:** do not certify the warning function; restore original firmware if needed.
- **Viewer stutters:** use 2D Parking Assist *schematic* for safety checks; high-poly Saga may take longer to load.

## Suggested FYP evidence
Record a single continuous bench demo of boot, 4 pods, five alert conditions, automatic cloud status, temporary offline test, and model switching. Keep short screenshots of the serial log (with credentials redacted), Dashboard connection status, four sensor distances and output warning indicators. This is an educational prototype, not a certified vehicle speed limiter.
