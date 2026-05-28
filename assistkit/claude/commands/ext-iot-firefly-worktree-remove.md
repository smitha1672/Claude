Remove a git worktree for a Jira task after the branch is merged.

Repo: git@github.com:KeepTruckin/iot-firefly-ext.git
Main repo path: /home/smith/workspace/code/iot-firefly-ext
Worktree root:  /home/smith/workspace/code_worktree/mirror

Usage:
  /ext-iot-firefly-worktree-remove <type> <JIRA-ID> <short-description>

  type:              feature | bugfix | hotfix | mirror | none
  short-description: the slug after the JIRA-ID in the local branch name (e.g. fix-parser)

Steps:
1. Remove the worktree:
   git -C /home/smith/workspace/code/iot-firefly-ext worktree remove /home/smith/workspace/code_worktree/mirror/<JIRA-ID>
   # add --force if the worktree has uncommitted changes

2. Delete the local branch:
   feature/bugfix/hotfix/mirror: git -C /home/smith/workspace/code/iot-firefly-ext branch -d <type>/<JIRA-ID>-<short-description>
   none:                  git -C /home/smith/workspace/code/iot-firefly-ext branch -d <JIRA-ID>-<short-description>

3. Prune stale references:
   git -C /home/smith/workspace/code/iot-firefly-ext worktree prune

4. Confirm:
   git -C /home/smith/workspace/code/iot-firefly-ext worktree list

Examples:
  /ext-iot-firefly-worktree-remove feature IOT-456 fix-parser
    → removes /home/smith/workspace/code_worktree/mirror/IOT-456

  /ext-iot-firefly-worktree-remove bugfix FIRE-789 fix-issue
    → removes /home/smith/workspace/code_worktree/mirror/FIRE-789

  /ext-iot-firefly-worktree-remove none AGWW-63 pmx-release
    → removes /home/smith/workspace/code_worktree/mirror/AGWW-63
