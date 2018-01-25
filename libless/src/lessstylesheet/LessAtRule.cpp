#include "less/lessstylesheet/LessAtRule.h"
#include "less/lessstylesheet/LessStylesheet.h"

LessAtRule::LessAtRule(const Token &keyword) : AtRule(keyword) {
}
LessAtRule::~LessAtRule() {
}

void LessAtRule::setLessStylesheet(LessStylesheet &s) {
  lessStylesheet = &s;
}

LessStylesheet *LessAtRule::getLessStylesheet() {
  return lessStylesheet;
}

void LessAtRule::process(Stylesheet &s, void* context) const {
  AtRule *target = s.createAtRule(getKeyword());

  target->setRule(getRule());

  ((ProcessingContext*)context)->processValue(target->getRule());
}

void LessAtRule::process(Ruleset &r, void* context) const {
  // Can't add @-rules to rulesets so ignore the statement.

}
