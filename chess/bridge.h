// JSON bridge interface for the chess engine.
// Accepts JSON command strings, returns JSON response strings.

#ifndef CHESS_BRIDGE_H
#define CHESS_BRIDGE_H

#include <memory>
#include <string>

#include "chess.h"
#include <nlohmann/json.hpp>

/**
 * Holds the bridge session state (the current game).
 * handleBridgeCommand() operates on this context.
 */
struct BridgeContext {
    std::unique_ptr<ChessGame> game;
};

/**
 * Process a single JSON bridge command and return a JSON response.
 *
 * Protocol:
 *   Input:  {"command":"X", ...params}
 *   Output: {"ok":true, ...data} or {"ok":false, "error":"..."}
 *
 * Commands: new_game, from_fen, make_move, get_state, parse_san, quit.
 *
 * Returns: JSON response string. For "quit", returns the response and sets
 *          the should_quit output parameter to true.
 */
std::string handleBridgeCommand(const std::string& input, BridgeContext& ctx, bool& should_quit);

/**
 * Run the JSON bridge main loop: read JSON lines from stdin, write responses to stdout.
 */
void runBridgeLoop();

#endif  // CHESS_BRIDGE_H
