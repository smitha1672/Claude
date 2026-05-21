Mirror an existing remote branch into a new git worktree.

Repo: git@github.com:KeepTruckin/kt.git
Main repo path: /home/smith/workspace/code/KeepTruckin/kt
Worktree root:  /home/smith/workspace/code_worktree

Usage:
  /kt-worktree-mirror <remote-branch>

  remote-branch: the branch name as it exists on upstream (e.g. feature/DEVPRD-456-fix-fleet-parser)

Steps:

1. Extract the JIRA-ID from the branch name (pattern: [A-Z]+-[0-9]+).
   If none is found, ask the user to provide a JIRA-ID for the worktree path.

2. Fetch from upstream:
   git -C /home/smith/workspace/code/KeepTruckin/kt fetch upstream

3a. If the local branch does not yet exist, create it tracking the remote:
    git -C /home/smith/workspace/code/KeepTruckin/kt worktree add --track \
      -b <remote-branch> \
      /home/smith/workspace/code_worktree/<JIRA-ID>/kt \
      upstream/<remote-branch>

3b. If the local branch already exists, check it out directly:
    git -C /home/smith/workspace/code/KeepTruckin/kt worktree add \
      /home/smith/workspace/code_worktree/<JIRA-ID>/kt \
      <remote-branch>

4. Confirm:
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree list

Examples:
  /kt-worktree-mirror feature/DEVPRD-456-fix-fleet-parser
  /kt-worktree-mirror bugfix/ABC-789-fix-issue
  /kt-worktree-mirror DEVPRD-123-my-feature
