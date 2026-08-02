// Copyright 2026 GridWhale.
//
// SPDX-License-Identifier: MIT

#ifndef BOCFEL_SESSION_H
#define BOCFEL_SESSION_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

enum class BocfelSessionState {
    Finished,
    WaitingForLine,
    WaitingForChar,
    WaitingForSave,
    WaitingForRestore,
    Error,
};

struct BocfelSessionResult {
    BocfelSessionState state = BocfelSessionState::Error;
    std::string output;
    std::string error;
    std::vector<uint8_t> save_data;
};

struct BocfelSessionOptions {
    bool suppress_prompt_marker = true;
};

struct BocfelSession;

BocfelSession *bocfel_session_create(const uint8_t *story_data, size_t story_size, BocfelSessionResult *result);
BocfelSession *bocfel_session_create_with_options(const uint8_t *story_data, size_t story_size, const BocfelSessionOptions *options, BocfelSessionResult *result);
BocfelSessionResult bocfel_session_process(BocfelSession *session, const char *input);
BocfelSessionResult bocfel_session_complete_save(BocfelSession *session, bool success);
BocfelSessionResult bocfel_session_restore(BocfelSession *session, const uint8_t *save_data, size_t save_size);
void bocfel_session_destroy(BocfelSession *session);

#endif
