import os

from dotenv import load_dotenv

load_dotenv()

BENCHCTL_BIN = os.getenv("BENCHCTL_BIN", "benchctl")
BENCHCTL_CONFIG = os.getenv("BENCHCTL_CONFIG", "")
