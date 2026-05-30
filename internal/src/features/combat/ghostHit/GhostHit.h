// GhostHit — independent swept-collision check + synthetic PLAYERHIT.
//
// RotMG's hit detection runs CLIENT-SIDE: the game's HBEAKBIHANL update tick
// checks bullet/player overlap and, if it fires, sends an outbound PLAYERHIT
// packet which the server then trusts to apply damage. AutoNexus and the
// damage tracker key off that PLAYERHIT.
//
// When SpeedHack alters Time.deltaTime or our NativeMoveTo moves the player
// faster than the game's per-tick collision can resolve, fast bullets can
// "phase through" the player: at frame N the bullet is in front of us, at
// frame N+1 it's behind us, no overlap was ever sampled. The game emits no
// PLAYERHIT. But the server runs its own integration in some cases and
// applies damage anyway → NEWTICK shows lower HP with no preceding packet
// → AutoNexus had no pre-damage signal and reacts only after the drop is
// already on the wire. This is the "ghost hit" pattern.
//
// GhostHit closes the gap by doing our OWN swept-segment collision check
// every game-update tick, against the SAME prediction (ComputePosAtSafe)
// the dodge uses, but on the player's actual position rather than the
// dodge's planning grid. When we detect a hit the game would have missed,
// we emit a signed event to the proxy via IpcBridge; the proxy crafts a
// PLAYERHIT packet on our behalf, which the server applies and which the
// in-process Auto Nexus plugin picks up — giving Auto Nexus the pre-damage
// signal it needs.
//
// On by default — ghost-hit deaths are far more harmful than the small
// theoretical detectability cost of synthetic packets. Can be disabled
// from the dashboard if a specific server starts flagging them.

#pragma once

namespace GhostHit {

// Per-tick check. Call from the game-update thread (Detour_AppEngineUpdate)
// alongside the other dodge helpers. Cheap when disabled (one atomic load),
// O(live-bullets) when on.
void Tick(void* player, float playerX, float playerY);

// Feature toggle. Wired from the dashboard via xdodgeGhostHit IPC key.
void SetEnabled(bool en);
bool IsEnabled();

}  // namespace GhostHit
