# Project instructions

Read and follow [AGENTS.md](../AGENTS.md) at the repository root before working.
Read [the five-phase roadmap](../docs/ROADMAP.md) and
[shared provider design](../docs/custom-dashboards/data-providers.md) before making
architectural changes. Document future Android integration; do not implement its
communication in Phase 2. Vehicle and future Android providers share manager/widgets.
Phase 2 scope is "Real Vehicle Data Integration": real MMI data in the Virtual
Cockpit; mock data only for tests. See [requirements](../docs/custom-dashboards/requirements.md).
Phase 2 is not complete; the existing work is a mock-only host foundation.
Use [the continuation guide](../docs/custom-dashboards/HANDOFF.md) to recover context.
Read [the current checkpoint](../docs/custom-dashboards/audit/CURRENT.md) and recent
[audit events](../docs/custom-dashboards/audit/LOG.md); follow the audit protocol to
record your RESUME event, milestones and handoff before context exhaustion.
AGENTS.md and the active checkpoint define scope. The user authorized publishing
all existing work to the fork's master, including P2-01/P2-02 and MAINT-141.
That consolidation is published; retain its test evidence and audit trail.
P2-03 was later merged to master on request. P2-04, the final
testing stage, now complete. The user then said: do not run any test (or write
test code) until the final request to implement and run tests (see AGENTS.md).
Merges/publication need their own approval. No release, vehicle action or
Android communication is authorized. Preserve the roadmap and dashboards.
