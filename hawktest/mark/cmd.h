// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <hawktest/cmark/cmark.h>

#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace md_test {

    /// One `@htest.*@` command taken from a heading, a list item, or a fence info string.
    struct Command {
        std::string tag;
        std::string name;
        std::vector<std::string> args;
        std::vector<std::string> codes;
        std::string link_url;
        std::string format;
        std::string body;
        int line{0};
        std::vector<Command> children;
    };

    /// ATX heading level, or empty when the node is not an ATX heading.
    std::optional<int> atx_heading_level(cmark_node *node, const std::string &markdown);

    /// Heading command, currently `@htest.suite@` plus its code span.
    std::optional<Command> parse_heading_command(cmark_node *heading);

    /// Fence command. `format` is the language word before the tag. `body` is the fence literal.
    std::optional<Command> parse_fence_command(cmark_node *code_block);

    /// List-item command. Args are the words after the tag. A link's url is `link_url`.
    std::optional<Command> parse_item_command(cmark_node *item);

    struct Kv {
        std::string key;
        std::string value;
    };

    /// `!load` is either a file link (`kUri`) or a name to look up (`kRef`).
    struct LoadExpr {
        enum class Kind { kUri, kRef };

        Kind kind{Kind::kRef};
        std::string key;
        std::string uri;
    };

    std::optional<Kv> parse_set(const Command &command);

    std::optional<std::string> parse_drop(const Command &command);

    std::optional<LoadExpr> parse_load(const Command &command);

    struct EnvCommand {
        enum class Action { kSet, kDrop, kLoad };

        Action action{Action::kSet};
        Kv set;
        std::string drop_key;
        LoadExpr load;
        int line{0};
    };

    struct LabelCommand {
        enum class Action { kDefine, kExpose, kLoad };

        Action action{Action::kDefine};
        std::string local_name;
        std::string format;
        std::string body;
        LoadExpr load;
        int line{0};
    };

    /// `@htest.env@` dispatched through `!set`, `!drop`, or `!load`.
    std::optional<EnvCommand> parse_env(const Command &command);

    /// `@htest.label@` and `@htest.expose@`, including a `!load` on a label.
    std::optional<LabelCommand> parse_label(const Command &command);

    /// One side of a match: `!load` (file or name) or `!label` (local name).
    struct MatchSide {
        std::string role;
        LoadExpr source;
        int line{0};
    };

    /// `@htest.match@ ordered` with nested `input` and `expect` commands.
    struct MatchCommand {
        std::string mode;
        MatchSide input;
        MatchSide expect;
        bool has_input{false};
        bool has_expect{false};
        int line{0};
    };

    std::optional<MatchCommand> parse_match(const Command &command);

    /// `@htest.flag@` with two code spans: name and value. Name has no `htest_` prefix.
    struct FlagCommand {
        std::string name;
        std::string value;
        int line{0};
    };

    std::optional<FlagCommand> parse_flag(const Command &command);

    /// `@htest.skip@` with a reason in a code span or the following words.
    struct SkipCommand {
        std::string reason;
        int line{0};
    };

    std::optional<SkipCommand> parse_skip(const Command &command);

}  // namespace md_test
