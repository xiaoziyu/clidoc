
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>
#include <list>

#include "ast/ast_node_interface.h"
#include "ast/string_utils.h"
#include "ast/tokenizer.h"
#include "code_gen/cpp/argv_processor.h"

using std::string;
using std::vector;
using std::list;
using std::set;
using std::find;
using std::next;
using std::copy;
using std::back_inserter;

namespace clidoc {

void ArgvProcessLogic::InsertSpace() {
  StringUtils::InsertSpace(
      {"(-- (.|\n)*$)"},  // excluded string after `--`.
      {"=", ","},         // insert space arround `=` and `,`.
      &argv_);
}

void ArgvProcessLogic::TokenizeArgv() {
  auto tokens = FromString(argv_);
  copy(tokens.begin(), tokens.end(), back_inserter(tokens_));
}

void ArgvProcessLogic::HandleDoubleHyphen() {
  auto pos_iter = find(tokens_.begin(), tokens_.end(),
                       Token(TerminalType::K_DOUBLE_HYPHEN));
  if (pos_iter != tokens_.end()) {
    // `--` would be removed in `HandleGroupedOptions`.
    for (auto iter = next(pos_iter);
         iter != tokens_.end(); ++iter) {
      iter->set_type(TerminalType::GENERAL_ELEMENT);
    }
  }
}

void ArgvProcessLogic::HandleGroupedOptions(
    const set<Token> &focused_bound_options) {
  auto end_iter = find(tokens_.begin(), tokens_.end(),
                       Token(TerminalType::K_DOUBLE_HYPHEN));
  if (end_iter != tokens_.end()) {
    // remove `--`.
    end_iter = tokens_.erase(end_iter);
  }
  for (auto token_iter = tokens_.begin();
       token_iter != end_iter; ++token_iter) {
    // consider two kinds of tokens:
    // 1. `GENERAL_ELEMENT` starts with `-`.
    // 2. `GROUPED_OPTIONS`.
    if (token_iter->type() == TerminalType::GENERAL_ELEMENT) {
      auto value = token_iter->value();
      if (value.front() != '-') {
        continue;
      }
    } else if (token_iter->type() != TerminalType::GROUPED_OPTIONS) {
      continue;
    }
    auto value = token_iter->value();
    for (auto char_iter = value.begin() + 1;
         char_iter != value.end(); ++char_iter) {
      auto option = Token(TerminalType::POSIX_OPTION,
                          string("-") + *char_iter);
      tokens_.insert(token_iter, option);
      if (focused_bound_options.find(option) == focused_bound_options.end()) {
        // option not bound.
        continue;
      } else {
        string option_argument_value(char_iter + 1, value.end());
        auto option_argument = Token(
            TerminalType::GENERAL_ELEMENT,
            option_argument_value);
        if (!option_argument_value.empty()) {
          tokens_.insert(token_iter, option_argument);
        }
        break;
      }
    }
    // remove original token.
    token_iter = tokens_.erase(token_iter);
    --token_iter;
  }
}

void ArgvProcessor::LoadArgv(const int &argc, const char **argv) {
  for (int index = 1; index != argc; ++index) {
    argv_.append(argv[index]);
    argv_.push_back(' ');
  }
}

vector<Token> ArgvProcessor::GetPreprocessedArguments(
    const set<Token> &focused_bound_options) const {
  ArgvProcessLogic process_logic(argv_);
  process_logic.InsertSpace();
  process_logic.TokenizeArgv();
  process_logic.HandleDoubleHyphen();
  process_logic.HandleGroupedOptions(focused_bound_options);
  return vector<Token>(
      process_logic.tokens_.begin(),
      process_logic.tokens_.end());
}

}  // namespace clidoc