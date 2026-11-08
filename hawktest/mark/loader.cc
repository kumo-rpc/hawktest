// Copyright (C) 2026 Kumo inc. and its affiliates. All Rights Reserved.

#include <hawktest/mark/loader.h>

#include <hawktest/cmark/cmark.h>
#include <hawktest/common/environment.h>
#include <hawktest/mark/cmd.h>

#include <fstream>
#include <sstream>

namespace md_test {
namespace {

bool read_file(const std::filesystem::path &path, std::string *bytes, std::string *error) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        *error = "cannot read " + path.string();
        return false;
    }
    std::ostringstream buffer;
    buffer << in.rdbuf();
    *bytes = buffer.str();
    return true;
}

bool add_label(hawk::LabelRegistry *labels, const std::string &prefix, const std::string &local, std::string format, std::string bytes, int line, int content_line, std::string *error) {
    if (prefix.empty() || local.empty() || local.find('.') != std::string::npos) {
        *error = "invalid label at line " + std::to_string(line);
        return false;
    }
    hawk::Label label;
    label.format = std::move(format);
    label.bytes = std::move(bytes);
    label.md_line = content_line;
    const std::string full_name = hawk::label_full_name(prefix, local);
    if (!labels->add(full_name, std::move(label))) {
        *error = "duplicate label " + full_name;
        return false;
    }
    return true;
}

std::string full_key(const std::string &prefix, const std::string &name) {
    if (name.find('.') != std::string::npos) {
        return name;
    }
    return prefix + "." + name;
}

bool remember_label(SuiteFields *fields, const std::string &key) {
    if (fields == nullptr || key.empty()) {
        return false;
    }
    fields->labels.push_back(key);
    return true;
}

bool read_label_file(hawk::LabelRegistry *labels, SuiteFields *fields, const std::filesystem::path &base, const std::string &prefix, const std::string &local, const std::string &uri, int line, std::string *error) {
    const std::string full = hawk::label_full_name(prefix, local);
    if (labels->find(full) != nullptr) {
        return remember_label(fields, local);
    }
    std::string bytes;
    if (!read_file(base / uri, &bytes, error)) {
        return false;
    }
    if (!add_label(labels, prefix, local, "file", std::move(bytes), line, 0, error)) {
        return false;
    }
    return remember_label(fields, local);
}

}  // namespace

bool load_mark_file(const std::filesystem::path &path, SuiteRegistry &suites, hawk::LabelRegistry &labels, std::string *error) {
    std::string message;
    if (error == nullptr) {
        error = &message;
    }
    error->clear();

    std::string markdown;
    if (!read_file(path, &markdown, error)) {
        return false;
    }
    const std::filesystem::path base = std::filesystem::absolute(path).parent_path();

    cmark_parser *parser = cmark_parser_new(CMARK_OPT_SOURCEPOS);
    cmark_parser_feed(parser, markdown.data(), markdown.size());
    cmark_node *doc = cmark_parser_finish(parser);
    cmark_parser_free(parser);
    if (doc == nullptr) {
        *error = "cmark parse failed";
        return false;
    }

    enum class Section { kNone, kIgnore, kLive };

    Section section = Section::kNone;
    Suite current;
    bool in_sub = false;
    bool have_suite = false;

    auto fields = [&]() -> SuiteFields * {
        if (!have_suite || section != Section::kLive) {
            return nullptr;
        }
        if (in_sub) {
            return &current.sub_suites.back();
        }
        return &current;
    };

    auto prefix_of = [&]() -> std::string {
        if (!have_suite || section != Section::kLive) {
            return {};
        }
        if (in_sub) {
            return current.sub_suites.back().prefix();
        }
        return current.name;
    };

    auto finish_suite = [&]() -> bool {
        if (!have_suite) {
            return true;
        }
        const std::string suite_name = current.name;
        if (!suites.add(std::move(current))) {
            *error = "duplicate suite " + suite_name;
            have_suite = false;
            return false;
        }
        current = Suite{};
        have_suite = false;
        in_sub = false;
        return true;
    };

    bool ok = true;
    for (cmark_node *block = cmark_node_first_child(doc); block != nullptr && ok; block = cmark_node_next(block)) {
        if (const std::optional<int> level = atx_heading_level(block, markdown)) {
            const std::optional<Command> command = parse_heading_command(block);
            const bool is_suite = command.has_value() && command->name == "suite" && !command->codes.empty();
            if (*level != 2 && *level != 3) {
                section = Section::kIgnore;
                in_sub = false;
                continue;
            }
            if (!is_suite) {
                section = Section::kIgnore;
                in_sub = false;
                continue;
            }
            if (*level == 2) {
                if (!finish_suite()) {
                    ok = false;
                    break;
                }
                current.name = command->codes.front();
                current.md_path = path;
                current.md_line = command->line;
                have_suite = true;
                in_sub = false;
                section = Section::kLive;
                continue;
            }
            if (!have_suite) {
                *error = "subsuite without suite at line " + std::to_string(command->line);
                ok = false;
                break;
            }
            SubSuite sub;
            sub.name = command->codes.front();
            sub.suite_name = current.name;
            sub.md_path = path;
            sub.md_line = command->line;
            current.sub_suites.push_back(std::move(sub));
            in_sub = true;
            section = Section::kLive;
            continue;
        }

        if (section != Section::kLive) {
            continue;
        }

        if (const std::optional<Command> command = parse_fence_command(block)) {
            const std::optional<LabelCommand> label = parse_label(*command);
            if (!label.has_value() || label->action != LabelCommand::Action::kDefine) {
                continue;
            }
            if (!add_label(&labels, prefix_of(), label->local_name, label->format, label->body, label->line, label->line + 1, error) ||
                !remember_label(fields(), label->local_name)) {
                ok = false;
            }
            continue;
        }

        if (cmark_node_get_type(block) != CMARK_NODE_LIST) {
            continue;
        }
        for (cmark_node *item = cmark_node_first_child(block); item != nullptr && ok; item = cmark_node_next(item)) {
            const std::optional<Command> command = parse_item_command(item);
            if (!command.has_value()) {
                continue;
            }
            if (const std::optional<SkipCommand> skip = parse_skip(*command)) {
                fields()->skip = true;
                fields()->skip_reason = skip->reason;
                continue;
            }
            if (command->name == "flag") {
                const std::optional<FlagCommand> flag = parse_flag(*command);
                if (!flag.has_value()) {
                    *error = "invalid flag at line " + std::to_string(command->line);
                    ok = false;
                    break;
                }
                if (!fields()->flags.emplace(flag->name, flag->value).second) {
                    *error = "duplicate flag " + flag->name + " at line " + std::to_string(flag->line);
                    ok = false;
                    break;
                }
                continue;
            }
            if (const std::optional<MatchCommand> match = parse_match(*command)) {
                if (!match->has_input || !match->has_expect) {
                    *error = "match needs input and expect at line " + std::to_string(match->line);
                    ok = false;
                    break;
                }
                auto store_side = [&](const LoadExpr &source, std::string *key) -> bool {
                    if (source.kind == LoadExpr::Kind::kUri) {
                        if (!read_label_file(&labels, fields(), base, prefix_of(), source.key, source.uri, match->line, error)) {
                            return false;
                        }
                    } else if (!remember_label(fields(), source.key)) {
                        return false;
                    }
                    *key = source.key;
                    return true;
                };
                Match stored;
                stored.mode = match->mode;
                stored.md_line = match->line;
                if (!store_side(match->input.source, &stored.input) || !store_side(match->expect.source, &stored.expect)) {
                    ok = false;
                    break;
                }
                fields()->matches.push_back(std::move(stored));
                continue;
            }
            if (const std::optional<EnvCommand> env = parse_env(*command)) {
                EnvBinding binding;
                const std::string prefix = prefix_of();
                if (env->action == EnvCommand::Action::kSet) {
                    binding.key = env->set.key;
                    hawk::env_registry().set(full_key(prefix, binding.key), env->set.value);
                } else if (env->action == EnvCommand::Action::kDrop) {
                    binding.key = env->drop_key;
                    binding.drop = true;
                } else if (env->load.kind == LoadExpr::Kind::kUri) {
                    binding.key = env->load.key;
                    std::string bytes;
                    if (!read_file(base / env->load.uri, &bytes, error)) {
                        ok = false;
                        break;
                    }
                    hawk::env_registry().set(full_key(prefix, binding.key), std::move(bytes));
                } else {
                    binding.source = full_key(prefix, env->load.key);
                    const std::size_t dot = binding.source.rfind('.');
                    binding.key = dot == std::string::npos ? binding.source : binding.source.substr(dot + 1);
                }
                fields()->env.push_back(std::move(binding));
                continue;
            }
            const std::optional<LabelCommand> label = parse_label(*command);
            if (!label.has_value()) {
                continue;
            }
            if (label->load.kind == LoadExpr::Kind::kRef) {
                if (!remember_label(fields(), label->local_name)) {
                    ok = false;
                }
                continue;
            }
            if (label->action == LabelCommand::Action::kDefine) {
                if (!add_label(&labels, prefix_of(), label->local_name, label->format, label->body, label->line, label->line + 1, error) ||
                    !remember_label(fields(), label->local_name)) {
                    ok = false;
                }
                continue;
            }
            if (!read_label_file(&labels, fields(), base, prefix_of(), label->local_name, label->load.uri, label->line, error)) {
                ok = false;
            }
        }
    }

    cmark_node_free(doc);
    if (!ok) {
        return false;
    }
    return finish_suite();
}

}  // namespace md_test
