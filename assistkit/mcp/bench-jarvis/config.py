import os

from dotenv import load_dotenv

load_dotenv()

BENCHCTL_BIN = os.getenv("BENCHCTL_BIN", "benchctl")
BENCHCTL_CONFIG = os.getenv("BENCHCTL_CONFIG", "")

# benchctl resolves its relative defaults (config/psu_profiles.toml,
# config/js220_profiles.toml, config/js220_config.json) against the process's
# working directory, not its install location. Run it from a fixed directory
# (typically the benchctl repo root) so those defaults resolve the same way
# regardless of where this MCP server was launched from.
BENCHCTL_CWD = os.getenv("BENCHCTL_CWD", "") or None
