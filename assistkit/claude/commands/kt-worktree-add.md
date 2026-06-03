Add a new git worktree for a Jira task.

Repo: git@github.com:KeepTruckin/kt.git
Main repo path: /home/smith/workspace/code/KeepTruckin/kt
Worktree root:  /home/smith/workspace/code_worktree

Branch naming rules:
  feature/JIRA-ID-short-description   ← new feature
  bugfix/JIRA-ID-short-description    ← bug fix
  hotfix/JIRA-ID-short-description    ← critical production fix
  mirror/JIRA-ID-short-description    ← mirrored branch
  JIRA-ID-short-description           ← general / no prefix

Usage:
  /worktree-add <type> <JIRA-ID> <short-description>
  /worktree-add <type> <JIRA-ID> <short-description> from <remote-branch>
  /worktree-add <type> <short-description>
  /worktree-add <type> <short-description> from <remote-branch>

  type:              feature | bugfix | hotfix | mirror | none
  JIRA-ID:           optional — if omitted, <short-description> is used as the worktree folder name and branch suffix
  short-description: the slug appended after the JIRA-ID in the branch name (e.g. fix-fleet-parser)

Cases:

1. New branch from HEAD:
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree add /home/smith/workspace/code_worktree/<type>/<JIRA-ID or short-description>/kt -b <type>/<JIRA-ID>-<short-description>
   # for none type: -b <JIRA-ID>-<short-description>  (no prefix)

   Examples:
   /worktree-add feature DEVPRD-456 fix-fleet-parser
   /worktree-add bugfix ABC-456 fix-issue
   /worktree-add hotfix PROD-789 critical-fix
   /worktree-add none DEVPRD-456 my-feature
   /worktree-add feature fix-fleet-parser          ← no JIRA-ID

2. New branch based on a remote branch:
   git -C /home/smith/workspace/code/KeepTruckin/kt fetch upstream
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree add /home/smith/workspace/code_worktree/<type>/<JIRA-ID or short-description>/kt -b <type>/<JIRA-ID>-<short-description> upstream/<remote-branch>
   # for none type: -b <JIRA-ID>-<short-description>  (no prefix)

   Example:
   /worktree-add feature DEVPRD-456 fix-fleet-parser from upstream/release-2.1

3. Check out remote branch as-is (no rename):
   git -C /home/smith/workspace/code/KeepTruckin/kt fetch upstream
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree add /home/smith/workspace/code_worktree/<type>/<JIRA-ID or short-description>/kt upstream/<remote-branch>

   Example:
   /worktree-add none DEVPRD-456 fix-fleet-parser from main

Branch name result per type:
  feature  → feature/DEVPRD-456-fix-fleet-parser  (or feature/fix-fleet-parser if no JIRA-ID)
  bugfix   → bugfix/DEVPRD-456-fix-fleet-parser   (or bugfix/fix-fleet-parser if no JIRA-ID)
  hotfix   → hotfix/DEVPRD-456-fix-fleet-parser   (or hotfix/fix-fleet-parser if no JIRA-ID)
  none     → DEVPRD-456-fix-fleet-parser           (or fix-fleet-parser if no JIRA-ID)

Confirm after any case:
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree list
