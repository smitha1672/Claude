from app import mcp

import tools.add      # noqa: F401
import tools.pool     # noqa: F401
import tools.search   # noqa: F401
import tools.explain  # noqa: F401
import tools.codegen  # noqa: F401
import tools.compare  # noqa: F401

if __name__ == "__main__":
    mcp.run()
