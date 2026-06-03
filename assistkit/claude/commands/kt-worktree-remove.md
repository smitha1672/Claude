Remove a git worktree for a Jira task after the branch is merged.

Repo: git@github.com:KeepTruckin/kt.git
Main repo path: /home/smith/workspace/code/KeepTruckin/kt
Worktree root:  /home/smith/workspace/code_worktree

Usage:
  /kt-worktree-remove <type> <JIRA-ID> <short-description>
  /kt-worktree-remove <type> <short-description>

  type:              feature | bugfix | hotfix | mirror | none
  JIRA-ID:           optional — if omitted, <short-description> was used as the worktree folder name and branch suffix
  short-description: the slug after the JIRA-ID in the local branch name (e.g. fix-fleet-parser)

Steps:
1. Remove the worktree:
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree remove /home/smith/workspace/code_worktree/<type>/<JIRA-ID or short-description>/kt
   # add --force if the worktree has uncommitted changes

2. Delete the local branch:
   feature/bugfix/hotfix/mirror: git -C /home/smith/workspace/code/KeepTruckin/kt branch -d <type>/<JIRA-ID>-<short-description>  (or <type>/<short-description> if no JIRA-ID)
   none:                         git -C /home/smith/workspace/code/KeepTruckin/kt branch -d <JIRA-ID>-<short-description>           (or <short-description> if no JIRA-ID)

3. Prune stale references:
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree prune

4. Confirm:
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree list

Examples:
  /kt-worktree-remove feature DEVPRD-456 fix-fleet-parser
  /kt-worktree-remove bugfix ABC-456 fix-issue
  /kt-worktree-remove hotfix PROD-789 critical-fix
  /kt-worktree-remove none DEVPRD-456 my-feature
