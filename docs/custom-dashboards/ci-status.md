# P2-C01: CI status and diagnosis

User request: "start C01". Workflow: [.github/workflows/tests.yml](../../.github/workflows/tests.yml)
("Tests": push, pull_request, workflow_dispatch; one `host` job on ubuntu-24.04).

## Diagnosis (2026-11-08)

| Evidence | Result |
| --- | --- |
| `list_workflows` (GitHub API) | "Tests", id 410626026, state **active**, registered 2026-11-07 |
| `list_workflow_runs` (GitHub API) | **total_count 0**: no run ever, although master and many branches were pushed after 2026-11-07 |
| Manual `workflow_dispatch` on master from this session | **403 "Resource not accessible by integration"**: the GitHub app used by the agent lacks permission to start workflows |

**Conclusion:** the workflow file is valid and registered, but GitHub Actions is not
executing workflows on this repository. Pushes would otherwise have created runs.
The most likely cause is the GitHub default for **forks** (this repository is a fork
of kamgurgul/mib2q-carplay): workflows stay disabled until the owner enables them
in the Actions tab. An organization/repository Actions policy is the other possible
cause. Confirming which needs the repository/organization settings page, which
the agent cannot read. The 403 is a separate limitation of the agent's token and
does not prove Actions is disabled.

**Owner action needed (one-time):**

1. Open the repository's **Actions** tab. If GitHub shows "Workflows aren't being
   run on this forked repository", choose **"I understand my workflows, go ahead
   and enable them"**.
2. If no such banner appears, check **Settings → Actions → General** (and the
   `alpersid-org` organization's Actions policy): allow actions and reusable
   workflows, at least the GitHub-authored `actions/checkout` and `actions/setup-java`.
3. Start a run: **Actions → Tests → Run workflow** on `master`, or push any commit.

After that, the agent can read the run, jobs and logs (read access works) and
repair real failures (plan step 3).

## Local workflow-equivalent run (local evidence, not CI)

Fresh clone of master `ab97450` in a scratch directory; same steps as the job: clang
18 as `cc`, `ksh`/`mksh`/`zlib1g-dev` installed, `SKIP_JAR_CHECK=1`, upstream tags
fetched. Host: Linux container, JDK 21 (the job uses Temurin 17).

| Workflow step | Local result |
| --- | --- |
| Host tests: `sh scripts/run_tests.sh` | 38 of 39 checks **PASS**; `hw301_bridge_tests` **UNAVAILABLE here**: the container has no IPv6 (`EAFNOSUPPORT`), GitHub runners are expected to have IPv6 loopback. Verified with a scratch copy omitting only that check: exit 0. |
| `make -C dashboard all test test-sanitize test-java` | **PASS** |
| `bash scripts/test_app_lifecycle.sh` | **PASS** |
| `python3 tests/installer_recovery_test.py` | **PASS** (10 tests) |
| upstream tag fetch + `python3 tests/upstream_v141_test.py` | **PASS** (4 tests) |

Environment notes for reproducing locally: the clang AddressSanitizer runtime
(`libclang-rt-18-dev`) must be installed; a default `apt-get install clang` pulls it
in as a recommended package, a minimal install does not. With gcc 13 instead of
clang, `alt111_settings_test` fails to compile (see validation.md); the workflow
deliberately uses clang for that reason.

## Acceptance (plan C01)

| Criterion | State |
| --- | --- |
| Determine whether Actions is enabled/discovering the workflow | Discovered and active; **not executing** (0 runs). Exact cause needs the owner's settings view. |
| Execute the workflow and inspect jobs/logs | **Blocked**: needs the owner to enable Actions (agent token cannot dispatch). |
| Local evidence recorded separately | Done (table above). |
| Successful observed CI run for the revision | **Unresolved** until a run exists; not claimed. |
