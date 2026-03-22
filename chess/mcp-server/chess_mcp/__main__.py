"""CLI entry point for the chess MCP server.

Usage:
    python -m chess_mcp [--port PORT] [--engine PATH] [--seed N] [--record PATH]
"""

from __future__ import annotations

import argparse
import asyncio
import sys

from .server import initialize, mcp, shutdown


def main() -> None:
    parser = argparse.ArgumentParser(description="Chess MCP Server")
    parser.add_argument("--port", type=int, default=8000, help="Port to listen on")
    parser.add_argument("--engine", type=str, default=None, help="Path to chess engine binary")
    parser.add_argument("--seed", type=int, default=None, help="Seed for deterministic mode")
    parser.add_argument("--record", type=str, default=None, help="Path to record game log (JSONL)")
    args = parser.parse_args()

    async def run() -> None:
        await initialize(engine_path=args.engine, seed=args.seed, record_path=args.record)
        try:
            # Run with streamable HTTP transport
            await mcp.run_async(transport="sse")
        finally:
            await shutdown()

    try:
        asyncio.run(run())
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()
