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
//

#include <hawktest/analysis/failure_reason.h>

namespace hawk {


    namespace {

        // The string representation of the values received in EqFailure() are already
        // escaped. Split them on escaped '\n' boundaries. Leave all other escaped
        // characters the same.
        std::vector<std::string> SplitEscapedString(const std::string& str) {
            std::vector<std::string> lines;
            size_t start = 0, end = str.size();
            if (end > 2 && str[0] == '"' && str[end - 1] == '"') {
                ++start;
                --end;
            }
            bool escaped = false;
            for (size_t i = start; i + 1 < end; ++i) {
                if (escaped) {
                    escaped = false;
                    if (str[i] == 'n') {
                        lines.push_back(str.substr(start, i - start - 1));
                        start = i + 1;
                    }
                } else {
                    escaped = str[i] == '\\';
                }
            }
            lines.push_back(str.substr(start, end - start));
            return lines;
        }

    } // namespace


        namespace edit_distance {
            std::vector<EditType> CalculateOptimalEdits(const std::vector<size_t>& left,
                const std::vector<size_t>& right) {
                std::vector<std::vector<double>> costs(
                    left.size() + 1, std::vector<double>(right.size() + 1));
                std::vector<std::vector<EditType>> best_move(
                    left.size() + 1, std::vector<EditType>(right.size() + 1));

                // Populate for empty right.
                for (size_t l_i = 0; l_i < costs.size(); ++l_i) {
                    costs[l_i][0] = static_cast<double>(l_i);
                    best_move[l_i][0] = kRemove;
                }
                // Populate for empty left.
                for (size_t r_i = 1; r_i < costs[0].size(); ++r_i) {
                    costs[0][r_i] = static_cast<double>(r_i);
                    best_move[0][r_i] = kAdd;
                }

                for (size_t l_i = 0; l_i < left.size(); ++l_i) {
                    for (size_t r_i = 0; r_i < right.size(); ++r_i) {
                        if (left[l_i] == right[r_i]) {
                            // Found a match. Consume it.
                            costs[l_i + 1][r_i + 1] = costs[l_i][r_i];
                            best_move[l_i + 1][r_i + 1] = kMatch;
                            continue;
                        }

                        const double add = costs[l_i + 1][r_i];
                        const double remove = costs[l_i][r_i + 1];
                        const double replace = costs[l_i][r_i];
                        if (add < remove && add < replace) {
                            costs[l_i + 1][r_i + 1] = add + 1;
                            best_move[l_i + 1][r_i + 1] = kAdd;
                        } else if (remove < add && remove < replace) {
                            costs[l_i + 1][r_i + 1] = remove + 1;
                            best_move[l_i + 1][r_i + 1] = kRemove;
                        } else {
                            // We make replace a little more expensive than add/remove to lower
                            // their priority.
                            costs[l_i + 1][r_i + 1] = replace + 1.00001;
                            best_move[l_i + 1][r_i + 1] = kReplace;
                        }
                    }
                }

                // Reconstruct the best path. We do it in reverse order.
                std::vector<EditType> best_path;
                for (size_t l_i = left.size(), r_i = right.size(); l_i > 0 || r_i > 0;) {
                    EditType move = best_move[l_i][r_i];
                    best_path.push_back(move);
                    l_i -= move != kAdd;
                    r_i -= move != kRemove;
                }
                std::reverse(best_path.begin(), best_path.end());
                return best_path;
            }

            namespace {

                // Helper class to convert string into ids with deduplication.
                class InternalStrings {
                public:
                    size_t GetId(const std::string& str) {
                        IdMap::iterator it = ids_.find(str);
                        if (it != ids_.end())
                            return it->second;
                        size_t id = ids_.size();
                        return ids_[str] = id;
                    }

                private:
                    typedef std::map<std::string, size_t> IdMap;
                    IdMap ids_;
                };

            } // namespace

            std::vector<EditType> CalculateOptimalEdits(
                const std::vector<std::string>& left,
                const std::vector<std::string>& right) {
                std::vector<size_t> left_ids, right_ids;
                {
                    InternalStrings intern_table;
                    for (size_t i = 0; i < left.size(); ++i) {
                        left_ids.push_back(intern_table.GetId(left[i]));
                    }
                    for (size_t i = 0; i < right.size(); ++i) {
                        right_ids.push_back(intern_table.GetId(right[i]));
                    }
                }
                return CalculateOptimalEdits(left_ids, right_ids);
            }

            namespace {

                // Helper class that holds the state for one hunk and prints it out to the
                // stream.
                // It reorders adds/removes when possible to group all removes before all
                // adds. It also adds the hunk header before printint into the stream.
                class Hunk {
                public:
                    Hunk(size_t left_start, size_t right_start)
                        : left_start_(left_start)
                        , right_start_(right_start)
                        , adds_()
                        , removes_()
                        , common_() { }

                    void PushLine(char edit, const char* line) {
                        switch (edit) {
                        case ' ':
                            ++common_;
                            FlushEdits();
                            hunk_.push_back(std::make_pair(' ', line));
                            break;
                        case '-':
                            ++removes_;
                            hunk_removes_.push_back(std::make_pair('-', line));
                            break;
                        case '+':
                            ++adds_;
                            hunk_adds_.push_back(std::make_pair('+', line));
                            break;
                        }
                    }

                    void PrintTo(std::ostream* os) {
                        PrintHeader(os);
                        FlushEdits();
                        for (std::list<std::pair<char, const char*>>::const_iterator it = hunk_.begin();
                            it != hunk_.end(); ++it) {
                            *os << it->first << it->second << "\n";
                        }
                    }

                    bool has_edits() const { return adds_ || removes_; }

                private:
                    void FlushEdits() {
                        hunk_.splice(hunk_.end(), hunk_removes_);
                        hunk_.splice(hunk_.end(), hunk_adds_);
                    }

                    // Print a unified diff header for one hunk.
                    // The format is
                    //   "@@ -<left_start>,<left_length> +<right_start>,<right_length> @@"
                    // where the left/right lengths are omitted if unnecessary.
                    void PrintHeader(std::ostream* ss) const {
                        size_t left_length = removes_ + common_;
                        size_t right_length = adds_ + common_;
                        *ss << "@@ " << "-" << left_start_;
                        if (left_length != 1) {
                            *ss << "," << left_length;
                        }
                        *ss << " " << "+" << right_start_;
                        if (right_length != 1) {
                            *ss << "," << right_length;
                        }
                        *ss << " @@\n";
                    }

                    size_t left_start_, right_start_;
                    size_t adds_, removes_, common_;
                    std::list<std::pair<char, const char*>> hunk_, hunk_adds_, hunk_removes_;
                };

            } // namespace

            // Create a list of diff hunks in Unified diff format.
            // Each hunk has a header generated by PrintHeader above plus a body with
            // lines prefixed with ' ' for no change, '-' for deletion and '+' for
            // addition.
            // 'context' represents the desired unchanged prefix/suffix around the diff.
            // If two hunks are close enough that their contexts overlap, then they are
            // joined into one hunk.
            std::string CreateUnifiedDiff(const std::vector<std::string>& left,
                const std::vector<std::string>& right,
                size_t context) {
                const std::vector<EditType> edits = CalculateOptimalEdits(left, right);

                size_t l_i = 0, r_i = 0, edit_i = 0;
                std::stringstream ss;
                while (edit_i < edits.size()) {
                    // Find first edit.
                    while (edit_i < edits.size() && edits[edit_i] == kMatch) {
                        ++l_i;
                        ++r_i;
                        ++edit_i;
                    }

                    // Find the first line to include in the hunk.
                    const size_t prefix_context = std::min(l_i, context);
                    Hunk hunk(l_i - prefix_context + 1, r_i - prefix_context + 1);
                    for (size_t i = prefix_context; i > 0; --i) {
                        hunk.PushLine(' ', left[l_i - i].c_str());
                    }

                    // Iterate the edits until we found enough suffix for the hunk or the input
                    // is over.
                    size_t n_suffix = 0;
                    for (; edit_i < edits.size(); ++edit_i) {
                        if (n_suffix >= context) {
                            // Continue only if the next hunk is very close.
                            auto it = edits.begin() + static_cast<int>(edit_i);
                            while (it != edits.end() && *it == kMatch)
                                ++it;
                            if (it == edits.end() || static_cast<size_t>(it - edits.begin()) - edit_i >= context) {
                                // There is no next edit or it is too far away.
                                break;
                            }
                        }

                        EditType edit = edits[edit_i];
                        // Reset count when a non match is found.
                        n_suffix = edit == kMatch ? n_suffix + 1 : 0;

                        if (edit == kMatch || edit == kRemove || edit == kReplace) {
                            hunk.PushLine(edit == kMatch ? ' ' : '-', left[l_i].c_str());
                        }
                        if (edit == kAdd || edit == kReplace) {
                            hunk.PushLine('+', right[r_i].c_str());
                        }

                        // Advance indices, depending on edit type.
                        l_i += edit != kAdd;
                        r_i += edit != kRemove;
                    }

                    if (!hunk.has_edits()) {
                        // We are done. We don't want this hunk.
                        break;
                    }

                    hunk.PrintTo(&ss);
                }
                return ss.str();
            }

        } // namespace edit_distance


    // Constructs and returns the message for an equality assertion
    // (e.g. ASSERT_EQ, EXPECT_STREQ, etc) failure.
    //
    // The first four parameters are the expressions used in the assertion
    // and their values, as strings.  For example, for ASSERT_EQ(foo, bar)
    // where foo is 5 and bar is 6, we have:
    //
    //   lhs_expression: "foo"
    //   rhs_expression: "bar"
    //   lhs_value:      "5"
    //   rhs_value:      "6"
    //
    // The ignoring_case parameter is true if and only if the assertion is a
    // *_STRCASEEQ*.  When it's true, the string "Ignoring case" will
    // be inserted into the message.
    AssertionResult EqFailure(const char* lhs_expression,
        const char* rhs_expression,
        const std::string& lhs_value,
        const std::string& rhs_value, bool ignoring_case) {
        hawk::Message msg;
        msg << "Expected equality of these values:";
        msg << "\n  " << lhs_expression;
        if (lhs_value != lhs_expression) {
            msg << "\n    Which is: " << lhs_value;
        }
        msg << "\n  " << rhs_expression;
        if (rhs_value != rhs_expression) {
            msg << "\n    Which is: " << rhs_value;
        }

        if (ignoring_case) {
            msg << "\nIgnoring case";
        }

        if (!lhs_value.empty() && !rhs_value.empty()) {
            const std::vector<std::string> lhs_lines = SplitEscapedString(lhs_value);
            const std::vector<std::string> rhs_lines = SplitEscapedString(rhs_value);
            if (lhs_lines.size() > 1 || rhs_lines.size() > 1) {
                msg << "\nWith diff:\n"
                    << edit_distance::CreateUnifiedDiff(lhs_lines, rhs_lines);
            }
        }

        return AssertionFailure() << msg;
    }

    // Constructs a failure message for Boolean assertions such as EXPECT_TRUE.
    std::string GetBoolAssertionFailureMessage(
        const AssertionResult& assertion_result, const char* expression_text,
        const char* actual_predicate_value, const char* expected_predicate_value) {
        const char* actual_message = assertion_result.message();
        hawk::Message msg;
        msg << "Value of: " << expression_text
            << "\n  Actual: " << actual_predicate_value;
        if (actual_message[0] != '\0')
            msg << " (" << actual_message << ")";
        msg << "\nExpected: " << expected_predicate_value;
        return msg.GetString();
    }
}  // namespace hawk
