import $knot from "@knot/runtime";
function EmptyStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {};
};
export { EmptyStyle };
function NoArgsStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {};
};
export { NoArgsStyle };
function IdentifierRuleStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {
    ["#foo"]: {}
  };
};
export { IdentifierRuleStyle };
function ClassRuleStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {
    [".foo"]: {}
  };
};
export { ClassRuleStyle };
function MultipleRuleStyle($props$) {
  var $ = $knot.style.styleExpressionPlugin;
  var $rules$ = $knot.style.styleRulePlugin;
  return {
    ["#foo"]: {},
    [".bar"]: {}
  };
};
export { MultipleRuleStyle };
