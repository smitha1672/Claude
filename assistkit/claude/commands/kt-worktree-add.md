Add a new git worktree for a Jira task.

Repo: git@github.com:KeepTruckin/kt.git
Main repo path: /home/smith/workspace/code/KeepTruckin/kt
Worktree root:  /home/smith/workspace/code_worktree

Branch naming rules:
  feature/JIRA-ID-short-description   ← new feature
  bugfix/JIRA-ID-short-description    ← bug fix
  hotfix/JIRA-ID-short-description    ← critical production fix
  JIRA-ID-short-description           ← general / no prefix

Usage:
  /worktree-add <type> <JIRA-ID> <short-description>
  /worktree-add <type> <JIRA-ID> <short-description> from <remote-branch>

  type: feature | bugfix | hotfix | none

Cases:

1. New branch from HEAD:
   git worktree add /home/smith/workspace/code_worktree/$JIRA-ID/kt -b $type/$JIRA-ID-$short-description

   Examples:
   /worktree-add feature DEVPRD-456 fix-fleet-parser
   /worktree-add bugfix ABC-456 fix-issue
   /worktree-add hotfix PROD-789 critical-fix
   /worktree-add none DEVPRD-456 my-feature

2. New branch based on a remote branch:
   git fetch upstream
   git worktree add /home/smith/workspace/code_worktree/$JIRA-ID/kt -b $type/$JIRA-ID-$short-description upstream/$remote-branch

   Example:
   /worktree-add feature DEVPRD-456 fix-fleet-parser from upstream/release-2.1

3. Check out remote branch as-is (no rename):
   git fetch upstream
   git worktree add /home/smith/workspace/code_worktree/$JIRA-ID/kt upstream/$remote-branch

   Example:
   /worktree-add none DEVPRD-456 fix-fleet-parser from main

Branch name result per type:
  feature  → feature/DEVPRD-456-fix-fleet-parser
  bugfix   → bugfix/DEVPRD-456-fix-fleet-parser
  hotfix   → hotfix/DEVPRD-456-fix-fleet-parser
  none     → DEVPRD-456-fix-fleet-parser

Confirm after any case:
   git worktree list
