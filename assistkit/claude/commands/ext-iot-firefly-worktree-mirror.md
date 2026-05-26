Mirror an existing remote branch into a new git worktree.

Repo: git@github.com:KeepTruckin/iot-firefly-ext.git
Main repo path: /home/smith/workspace/code/iot-firefly-ext
Worktree root:  /home/smith/workspace/code_worktree/mirror

Usage:
  /ext-iot-firefly-mirror <remote-branch> [jira-id]

  remote-branch: the branch name as it exists on upstream (e.g. feature/IOT-456-fix-parser)
  jira-id:       optional — override or supply the JIRA-ID when the branch name has none

Steps:

1. Extract the JIRA-ID or GITHUB-ID from the branch name (pattern: [A-Z]+-[0-9]+).
   If a jira-id argument was provided, use it instead (even if one was found in the branch name).
   If none is found and no argument was given, ask the user to provide a JIRA-ID for the worktree path.

2. Fetch from upstream:
   git -C /home/smith/workspace/code/iot-firefly-ext fetch upstream

3a. If the local branch does not yet exist, create it tracking the remote:
    git -C /home/smith/workspace/code/iot-firefly-ext worktree add --track \
      -b <remote-branch> \
      /home/smith/workspace/code_worktree/mirror/<JIRA-ID> \
      upstream/<remote-branch>

3b. If the local branch already exists, check it out directly:
    git -C /home/smith/workspace/code/iot-firefly-ext worktree add \
      /home/smith/workspace/code_worktree/mirror/<JIRA-ID> \
      <remote-branch>

4. Confirm:
   git -C /home/smith/workspace/code/iot-firefly-ext worktree list

Examples:
  /ext-iot-firefly-mirror feature/IOT-456-fix-parser
    → /home/smith/workspace/code_worktree/mirror/IOT-456

  /ext-iot-firefly-mirror bugfix/FIRE-789-fix-issue
    → /home/smith/workspace/code_worktree/mirror/FIRE-789

  /ext-iot-firefly-mirror IOT-123-my-feature
    → /home/smith/workspace/code_worktree/mirror/IOT-123

  /ext-iot-firefly-mirror pmx_FF_v9.0.0 AGWW-63
    → /home/smith/workspace/code_worktree/mirror/AGWW-63
