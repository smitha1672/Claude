Navigate to the source directory for a given Jira ID and component name.

Repo: git@github.com:KeepTruckin/kt.git
Main repo path: /home/smith/workspace/code/KeepTruckin/kt
Worktree root: /home/smith/workspace/code_worktree

Usage:
  /jira <JIRA-ID> <component>

Context:
  @/home/smith/workspace/code_worktree

Steps:
1. Resolve the target path:
   /home/smith/workspace/code_worktree/$JIRA-ID/kt/src/embedded/$component

2. Check if the path exists:
   ls /home/smith/workspace/code_worktree/$JIRA-ID/kt/src/embedded/$component

3. If path exists — change into it and confirm:
   cd /home/smith/workspace/code_worktree/$JIRA-ID/kt/src/embedded/$component
   pwd

4. If path does not exist — report the missing path and stop:
   Path not found: /home/smith/workspace/code_worktree/$JIRA-ID/kt/src/embedded/$component

Examples:
  /jira AGWW-60 ff_ble
  → cd /home/smith/workspace/code_worktree/AGWW-60/kt/src/embedded/ff_ble

  /jira DEVPRD-456 can_bus
  → cd /home/smith/workspace/code_worktree/DEVPRD-456/kt/src/embedded/can_bus
