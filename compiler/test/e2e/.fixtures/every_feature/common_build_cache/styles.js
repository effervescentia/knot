var $knot = require("@knot/runtime");
function EmptyStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {};
};
exports.EmptyStyle = EmptyStyle;
function NoArgsStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {};
};
exports.NoArgsStyle = NoArgsStyle;
function IdentifierRuleStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {
    ["#foo"]: {}
  };
};
exports.IdentifierRuleStyle = IdentifierRuleStyle;
function ClassRuleStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {
    [".foo"]: {}
  };
};
exports.ClassRuleStyle = ClassRuleStyle;
function MultipleRuleStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {
    ["#foo"]: {},
    [".bar"]: {}
  };
};
exports.MultipleRuleStyle = MultipleRuleStyle;
