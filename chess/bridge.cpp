// JSON bridge implementation for the chess engine.

#include "bridge.h"

#include <iostream>
#include <string>

using json = nlohmann::json;

namespace {

// Convert the engine's toJson() output to a proper nlohmann::json object.
// The engine builds JSON manually as a string, so we parse it.
json gameStateToJson(const ChessGame& game) {
    return json::parse(game.toJson());
}

json makeError(const std::string& msg) {
    return {{"ok", false}, {"error", msg}};
}

json makeOk() {
    return {{"ok", true}};
}

json handleNewGame(BridgeContext& ctx) {
    ctx.game = std::make_unique<ChessGame>();
    json resp = makeOk();
    resp["state"] = gameStateToJson(*ctx.game);
    return resp;
}

json handleFromFen(BridgeContext& ctx, const json& cmd) {
    if (!cmd.contains("fen") || !cmd["fen"].is_string()) {
        return makeError("missing or invalid 'fen' parameter");
    }
    std::string fen = cmd["fen"];
    auto game = ChessGame::fromFen(fen);
    if (!game) {
        return makeError("invalid FEN string");
    }
    ctx.game = std::move(game);
    json resp = makeOk();
    resp["state"] = gameStateToJson(*ctx.game);
    return resp;
}

json handleMakeMove(BridgeContext& ctx, const json& cmd) {
    if (!ctx.game) {
        return makeError("no active game");
    }
    if (!cmd.contains("move") || !cmd["move"].is_string()) {
        return makeError("missing or invalid 'move' parameter");
    }
    std::string moveStr = cmd["move"];

    // Try SAN first, fall back to LAN (same logic as Main.cpp)
    ChessMove move = ctx.game->parseSan(moveStr);
    if (move.isEnd()) {
        // Validate LAN format before constructing ChessMove.
        // Canonical LAN: "a1b2" or with promotion "a7a8q" (no separator).
        // Also accept hyphenated/spaced forms ("a1-b2", "a1 b2") by stripping the separator.
        std::string lanStr = moveStr;
        if (lanStr.size() >= 3 && (lanStr[2] == '-' || lanStr[2] == ' ')) {
            lanStr.erase(2, 1);  // strip separator to canonical form
        }
        bool validLan = false;
        if (lanStr.size() >= 4 && lanStr.size() <= 5) {
            char f1 = lanStr[0], r1 = lanStr[1], f2 = lanStr[2], r2 = lanStr[3];
            validLan = (f1 >= 'a' && f1 <= 'h' && r1 >= '1' && r1 <= '8' &&
                        f2 >= 'a' && f2 <= 'h' && r2 >= '1' && r2 <= '8');
        }
        if (!validLan) {
            return makeError("illegal or invalid move: " + moveStr);
        }
        move = ChessMove(lanStr.c_str());
    }

    if (!ctx.game->makeMove(move)) {
        return makeError("illegal or invalid move: " + moveStr);
    }

    json resp = makeOk();
    resp["state"] = gameStateToJson(*ctx.game);
    // Return the LAN of the move that was made
    resp["move_lan"] = std::string(move.toString());
    return resp;
}

json handleGetState(BridgeContext& ctx) {
    if (!ctx.game) {
        return makeError("no active game");
    }
    json resp = makeOk();
    resp["state"] = gameStateToJson(*ctx.game);
    return resp;
}

json handleParseSan(BridgeContext& ctx, const json& cmd) {
    if (!ctx.game) {
        return makeError("no active game");
    }
    if (!cmd.contains("san") || !cmd["san"].is_string()) {
        return makeError("missing or invalid 'san' parameter");
    }
    std::string san = cmd["san"];
    ChessMove move = ctx.game->parseSan(san);
    if (move.isEnd()) {
        return makeError("SAN not recognized: " + san);
    }
    json resp = makeOk();
    resp["lan"] = std::string(move.toString());
    return resp;
}

}  // namespace

std::string handleBridgeCommand(const std::string& input, BridgeContext& ctx, bool& should_quit) {
    should_quit = false;

    json cmd;
    try {
        cmd = json::parse(input);
    } catch (const json::parse_error& e) {
        return makeError(std::string("invalid JSON: ") + e.what()).dump();
    }

    if (!cmd.contains("command") || !cmd["command"].is_string()) {
        return makeError("missing or invalid 'command' field").dump();
    }

    std::string command = cmd["command"];

    json resp;
    if (command == "new_game") {
        resp = handleNewGame(ctx);
    } else if (command == "from_fen") {
        resp = handleFromFen(ctx, cmd);
    } else if (command == "make_move") {
        resp = handleMakeMove(ctx, cmd);
    } else if (command == "get_state") {
        resp = handleGetState(ctx);
    } else if (command == "parse_san") {
        resp = handleParseSan(ctx, cmd);
    } else if (command == "quit") {
        should_quit = true;
        resp = makeOk();
    } else {
        resp = makeError("unknown command: " + command);
    }

    return resp.dump();
}

void runBridgeLoop() {
    BridgeContext ctx;
    std::string line;

    while (std::getline(std::cin, line)) {
        bool quit = false;
        std::string response = handleBridgeCommand(line, ctx, quit);
        std::cout << response << std::endl;
        if (quit) {
            break;
        }
    }
}
