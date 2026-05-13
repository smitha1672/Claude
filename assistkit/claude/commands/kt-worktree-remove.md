Remove a git worktree for a Jira task after the branch is merged.

Repo: git@github.com:KeepTruckin/kt.git
Main repo path: /home/smith/workspace/code/KeepTruckin/kt
Worktree root:  /home/smith/workspace/code_worktree

Usage:
  /kt-worktree-remove <type> <JIRA-ID> <short-description>

  type: feature | bugfix | hotfix | none

Steps:
1. Remove the worktree:
   git worktree remove /home/smith/workspace/code_worktree/$JIRA-ID/kt

2. Delete the local branch:
   feature/bugfix/hotfix: git branch -d $type/$JIRA-ID-$short-description
   none:                  git branch -d $JIRA-ID-$short-description

3. Prune stale references:
   git worktree prune

Examples:
  /kt-worktree-remove feature DEVPRD-456 fix-fleet-parser
  /kt-worktree-remove bugfix ABC-456 fix-issue
  /kt-worktree-remove hotfix PROD-789 critical-fix
  /kt-worktree-remove none DEVPRD-456 my-feature

Confirm after:
  git worktree list
