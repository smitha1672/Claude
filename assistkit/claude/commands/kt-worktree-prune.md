Clean up stale git worktree references for the kt repo.

Repo: git@github.com:KeepTruckin/kt.git
Main repo path: /home/smith/workspace/code/KeepTruckin/kt
Worktree root:  /home/smith/workspace/code_worktree

Usage:
  /kt-worktree-prune

When to use:
  - After manually deleting a worktree folder without using git worktree remove
  - After a worktree path becomes invalid or unreachable

Steps:
1. Prune stale references:
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree prune

2. Confirm remaining worktrees:
   git -C /home/smith/workspace/code/KeepTruckin/kt worktree list
