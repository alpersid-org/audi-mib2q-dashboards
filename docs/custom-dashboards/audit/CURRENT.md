# Current agent checkpoint

Updated: 2026-11-07T20:44:16Z. Agent: Claude Code.
Session: `20261107T200748Z-claude`.

## Phase 2 scope (authoritative)

"Real Vehicle Data Integration": working Compact/Extended dashboards inside the
Virtual Cockpit showing real data from existing MMI systems; mock data only for
tests/development; unverified signals omitted; no Android-app dependency.
Full text and gap table: [requirements.md](../requirements.md#phase-2-scope--real-vehicle-data-integration-authoritative).
**Phase 2 is not complete.** P2-01..P2-05 (on master) are a mock-only host foundation.

## Active task and policies

- Task/state: `P2-07` (omit unverified widgets, scope req. 6/8) /
  `READY_FOR_REVIEW` on `feature/p2-07-verified-widgets`. DOC-SCOPE is on master.
- Done: trip/consumption removed; split navigation widgets; Extended redefined;
  Extended previews deleted. `make -C dashboard all` compiled (build only).
- Known: existing tests are out of date and will fail (list in
  widgets-p2-07.md); fix and rerun at the user's final test request.
- Testing policy (user): "do not run any test, there will be a ask at the end to
  implement and run the tests". No tests or new test code until then.
- Open user decisions: (1) authorize head-unit code (safety boundary currently
  forbids it); (2) choose the display surface; (3) supply/confirm vehicle
  evidence (firmware, adapter behaviour, signal availability during AA).

## Git state

- Branch `feature/p2-07-verified-widgets` from master f89a58c; master unchanged.

## Next action

Await the user: merge approval for P2-07, then PLAN step 8 (VehicleDataProvider)
and the open decisions (head-unit code, display surface, vehicle evidence).
Audit: [LOG.md](LOG.md).
