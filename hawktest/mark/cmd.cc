// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/mark/cmd.h>

#include <cctype>

namespace md_test {
namespace {

std::string trim(std::string_view text) {
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front()))) {
        text.remove_prefix(1);
    }
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back()))) {
        text.remove_suffix(1);
    }
    return std::string(text);
}

std::vector<std::string> split_words(std::string_view text) {
    std::vector<std::string> words;
    std::string current;
    for (const char ch : text) {
        if (std::isspace(static_cast<unsigned char>(ch))) {
            if (!current.empty()) {
                words.push_back(std::move(current));
                current.clear();
            }
            continue;
        }
        current.push_back(ch);
    }
    if (!current.empty()) {
        words.push_back(std::move(current));
    }
    return words;
}

std::string line_at(const std::string &markdown, int line_no) {
    int current = 1;
    std::size_t begin = 0;
    while (current < line_no && begin < markdown.size()) {
        if (markdown[begin++] == '\n') {
            ++current;
        }
    }
    const std::size_t end = markdown.find('\n', begin);
    if (end == std::string::npos) {
        return markdown.substr(begin);
    }
    return markdown.substr(begin, end - begin);
}

std::string command_name(std::string_view tag) {
    constexpr std::string_view kPrefix = "@htest.";
    constexpr std::string_view kSuffix = "@";
    if (tag.size() < kPrefix.size() + kSuffix.size() + 1 || tag.compare(0, kPrefix.size(), kPrefix) != 0 || tag.back() != '@') {
        return {};
    }
    return std::string(tag.substr(kPrefix.size(), tag.size() - kPrefix.size() - kSuffix.size()));
}

bool looks_like_tag(std::string_view word) {
    return word.size() > 2 && word.front() == '@' && word.back() == '@' && word.find("@htest.") == 0;
}

std::optional<Command> command_from_words(const std::vector<std::string> &words, int line) {
    std::size_t tag_at = words.size();
    for (std::size_t i = 0; i < words.size(); ++i) {
        if (looks_like_tag(words[i])) {
            tag_at = i;
            break;
        }
    }
    if (tag_at == words.size()) {
        return std::nullopt;
    }
    Command command;
    command.tag = words[tag_at];
    command.name = command_name(command.tag);
    command.line = line;
    if (tag_at > 0) {
        command.format = words[0];
    }
    for (std::size_t i = tag_at + 1; i < words.size(); ++i) {
        command.args.push_back(words[i]);
    }
    return command;
}

}  // namespace

std::optional<int> atx_heading_level(cmark_node *node, const std::string &markdown) {
    if (node == nullptr || cmark_node_get_type(node) != CMARK_NODE_HEADING) {
        return std::nullopt;
    }
    const std::string line = trim(line_at(markdown, cmark_node_get_start_line(node)));
    if (line.empty() || line.front() != '#') {
        return std::nullopt;
    }
    return cmark_node_get_heading_level(node);
}

std::optional<Command> parse_heading_command(cmark_node *heading) {
    if (heading == nullptr) {
        return std::nullopt;
    }
    std::string text;
    std::vector<std::string> codes;
    for (cmark_node *child = cmark_node_first_child(heading); child != nullptr; child = cmark_node_next(child)) {
        const char *literal = cmark_node_get_literal(child);
        if (literal == nullptr) {
            continue;
        }
        if (cmark_node_get_type(child) == CMARK_NODE_CODE) {
            codes.emplace_back(literal);
            continue;
        }
        text.append(literal);
        text.push_back(' ');
    }
    std::optional<Command> command = command_from_words(split_words(text), cmark_node_get_start_line(heading));
    if (!command.has_value()) {
        return std::nullopt;
    }
    command->codes = std::move(codes);
    return command;
}

std::optional<Command> parse_fence_command(cmark_node *code_block) {
    if (code_block == nullptr || cmark_node_get_type(code_block) != CMARK_NODE_CODE_BLOCK) {
        return std::nullopt;
    }
    const char *info = cmark_node_get_fence_info(code_block);
    if (info == nullptr) {
        return std::nullopt;
    }
    std::optional<Command> command = command_from_words(split_words(info), cmark_node_get_start_line(code_block));
    if (!command.has_value()) {
        return std::nullopt;
    }
    if (const char *literal = cmark_node_get_literal(code_block)) {
        command->body = literal;
    }
    return command;
}

namespace {

std::string join_tail(const std::vector<std::string> &args, std::size_t begin) {
    std::string value;
    for (std::size_t i = begin; i < args.size(); ++i) {
        if (!value.empty()) {
            value.push_back(' ');
        }
        value.append(args[i]);
    }
    return value;
}

bool is_verb(const Command &command, std::string_view verb) {
    return !command.args.empty() && command.args.front() == verb;
}

}  // namespace

std::optional<Kv> parse_set(const Command &command) {
    if (!is_verb(command, "!set") || command.args.size() < 2 || command.args[1].empty()) {
        return std::nullopt;
    }
    Kv kv;
    kv.key = command.args[1];
    kv.value = join_tail(command.args, 2);
    return kv;
}

std::optional<std::string> parse_drop(const Command &command) {
    if (!is_verb(command, "!drop") || command.args.size() != 2 || command.args[1].empty()) {
        return std::nullopt;
    }
    return command.args[1];
}

std::optional<LoadExpr> parse_load(const Command &command) {
    if (!is_verb(command, "!load")) {
        return std::nullopt;
    }
    LoadExpr load;
    if (!command.link_url.empty()) {
        load.kind = LoadExpr::Kind::kUri;
        load.uri = command.link_url;
        if (command.args.size() >= 2) {
            load.key = command.args[1];
        } else if (!command.codes.empty()) {
            load.key = command.codes.front();
        }
        if (load.key.empty()) {
            return std::nullopt;
        }
        return load;
    }
    load.kind = LoadExpr::Kind::kRef;
    if (!command.codes.empty()) {
        load.key = command.codes.front();
    } else if (command.args.size() >= 2) {
        load.key = command.args[1];
    }
    if (load.key.empty()) {
        return std::nullopt;
    }
    return load;
}

std::optional<EnvCommand> parse_env(const Command &command) {
    if (command.name != "env") {
        return std::nullopt;
    }
    EnvCommand env;
    env.line = command.line;
    if (const std::optional<Kv> set = parse_set(command)) {
        env.action = EnvCommand::Action::kSet;
        env.set = *set;
        return env;
    }
    if (const std::optional<std::string> key = parse_drop(command)) {
        env.action = EnvCommand::Action::kDrop;
        env.drop_key = *key;
        return env;
    }
    if (const std::optional<LoadExpr> load = parse_load(command)) {
        env.action = EnvCommand::Action::kLoad;
        env.load = *load;
        return env;
    }
    return std::nullopt;
}

std::optional<LabelCommand> parse_label(const Command &command) {
    LabelCommand label;
    label.line = command.line;
    label.format = command.format;
    label.body = command.body;
    if (command.name == "expose") {
        if (command.args.empty() || command.link_url.empty()) {
            return std::nullopt;
        }
        label.action = LabelCommand::Action::kExpose;
        label.local_name = command.args.front();
        label.load.kind = LoadExpr::Kind::kUri;
        label.load.key = label.local_name;
        label.load.uri = command.link_url;
        return label;
    }
    if (command.name != "label") {
        return std::nullopt;
    }
    if (const std::optional<LoadExpr> load = parse_load(command)) {
        label.action = LabelCommand::Action::kLoad;
        label.load = *load;
        label.local_name = load->key;
        return label;
    }
    if (command.args.empty()) {
        return std::nullopt;
    }
    label.action = LabelCommand::Action::kDefine;
    label.local_name = command.args.front();
    return label;
}

std::optional<Command> parse_item_command(cmark_node *item) {
    if (item == nullptr) {
        return std::nullopt;
    }
    std::string text;
    std::vector<std::string> codes;
    std::string link_url;
    for (cmark_node *child = cmark_node_first_child(item); child != nullptr; child = cmark_node_next(child)) {
        if (cmark_node_get_type(child) != CMARK_NODE_PARAGRAPH) {
            continue;
        }
        for (cmark_node *inline_node = cmark_node_first_child(child); inline_node != nullptr; inline_node = cmark_node_next(inline_node)) {
            const cmark_node_type type = cmark_node_get_type(inline_node);
            if (type == CMARK_NODE_LINK) {
                if (const char *url = cmark_node_get_url(inline_node)) {
                    link_url = url;
                }
                continue;
            }
            const char *literal = cmark_node_get_literal(inline_node);
            if (literal == nullptr) {
                continue;
            }
            if (type == CMARK_NODE_CODE) {
                codes.emplace_back(literal);
                continue;
            }
            text.append(literal);
        }
    }
    std::optional<Command> command = command_from_words(split_words(text), cmark_node_get_start_line(item));
    if (!command.has_value()) {
        return std::nullopt;
    }
    command->codes = std::move(codes);
    command->link_url = std::move(link_url);
    for (cmark_node *child = cmark_node_first_child(item); child != nullptr; child = cmark_node_next(child)) {
        if (cmark_node_get_type(child) != CMARK_NODE_LIST) {
            continue;
        }
        for (cmark_node *nested = cmark_node_first_child(child); nested != nullptr; nested = cmark_node_next(nested)) {
            if (std::optional<Command> nested_command = parse_item_command(nested)) {
                command->children.push_back(std::move(*nested_command));
            }
        }
    }
    return command;
}

namespace {

std::optional<LoadExpr> parse_label_ref(const Command &command) {
    if (command.args.empty() || command.args.front() != "!label") {
        return std::nullopt;
    }
    LoadExpr load;
    load.kind = LoadExpr::Kind::kRef;
    if (command.args.size() >= 2) {
        load.key = command.args[1];
    } else if (!command.codes.empty()) {
        load.key = command.codes.front();
    }
    if (load.key.empty()) {
        return std::nullopt;
    }
    return load;
}

}  // namespace

std::optional<MatchCommand> parse_match(const Command &command) {
    if (command.name != "match" || command.args.empty() || command.args.front().empty()) {
        return std::nullopt;
    }
    MatchCommand match;
    match.mode = command.args.front();
    match.line = command.line;
    for (const Command &child : command.children) {
        if (child.name != "input" && child.name != "expect") {
            continue;
        }
        std::optional<LoadExpr> source = parse_load(child);
        if (!source.has_value()) {
            source = parse_label_ref(child);
        }
        if (!source.has_value()) {
            continue;
        }
        MatchSide side;
        side.role = child.name;
        side.source = *source;
        side.line = child.line;
        if (child.name == "input") {
            match.input = std::move(side);
            match.has_input = true;
        } else {
            match.expect = std::move(side);
            match.has_expect = true;
        }
    }
    return match;
}

std::optional<FlagCommand> parse_flag(const Command &command) {
    if (command.name != "flag" || command.codes.size() != 2) {
        return std::nullopt;
    }
    if (command.codes[0].empty() || command.codes[0] == "filter" || command.codes[0] == "internal_run_death_test") {
        return std::nullopt;
    }
    FlagCommand flag;
    flag.name = command.codes[0];
    flag.value = command.codes[1];
    flag.line = command.line;
    return flag;
}

std::optional<SkipCommand> parse_skip(const Command &command) {
    if (command.name != "skip" && command.name != "case.skip") {
        return std::nullopt;
    }
    SkipCommand skip;
    skip.line = command.line;
    if (!command.codes.empty()) {
        skip.reason = command.codes.front();
    } else {
        for (const std::string &arg : command.args) {
            if (!skip.reason.empty()) {
                skip.reason.push_back(' ');
            }
            skip.reason.append(arg);
        }
    }
    if (skip.reason.empty()) {
        skip.reason = "skipped";
    }
    return skip;
}

}  // namespace md_test
