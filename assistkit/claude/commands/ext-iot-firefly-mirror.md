Mirror an existing remote branch into a new git worktree.

Repo: git@github.com:KeepTruckin/iot-firefly-ext.git
Main repo path: /home/smith/workspace/code/iot-firefly-ext
Worktree root:  /home/smith/workspace/code_worktree

Usage:
  /ext-iot-firefly-mirror <remote-branch>

  remote-branch: the branch name as it exists on upstream (e.g. feature/IOT-456-fix-parser)

Steps:

1. Extract the JIRA-ID from the branch name (pattern: [A-Z]+-[0-9]+).
   If none is found, ask the user to provide a JIRA-ID for the worktree path.

2. Fetch from upstream:
   git -C /home/smith/workspace/code/iot-firefly-ext fetch upstream

3a. If the local branch does not yet exist, create it tracking the remote:
    git -C /home/smith/workspace/code/iot-firefly-ext worktree add --track \
      -b <remote-branch> \
      /home/smith/workspace/code_worktree/<JIRA-ID>/iot-firefly-ext \
      upstream/<remote-branch>

3b. If the local branch already exists, check it out directly:
    git -C /home/smith/workspace/code/iot-firefly-ext worktree add \
      /home/smith/workspace/code_worktree/<JIRA-ID>/iot-firefly-ext \
      <remote-branch>

4. Confirm:
   git -C /home/smith/workspace/code/iot-firefly-ext worktree list

Examples:
  /ext-iot-firefly-mirror feature/IOT-456-fix-parser
  /ext-iot-firefly-mirror bugfix/FIRE-789-fix-issue
  /ext-iot-firefly-mirror IOT-123-my-feature
