var $knot = require("@knot/runtime");
function ExplicitNilView($props$) {
  return null;
};
exports.ExplicitNilView = ExplicitNilView;
function ImplicitNilView($props$) { };
exports.ImplicitNilView = ImplicitNilView;
function NoPropertiesView($props$) {
  return null;
};
exports.NoPropertiesView = NoPropertiesView;
function PropertiedView($props$) {
  var data_nil = $knot.platform.prop($props$, "data_nil");
  var data_true = $knot.platform.prop($props$, "data_true");
  var data_false = $knot.platform.prop($props$, "data_false");
  var data_positive_integer = $knot.platform.prop($props$,
                                                  "data_positive_integer");
  var data_negative_integer = $knot.platform.prop($props$,
                                                  "data_negative_integer");
  var data_positive_float = $knot.platform.prop($props$,
                                                "data_positive_float");
  var data_negative_float = $knot.platform.prop($props$,
                                                "data_negative_float");
  var data_string = $knot.platform.prop($props$, "data_string");
  var data_tag = $knot.platform.prop($props$, "data_tag");
  var data_fragment = $knot.platform.prop($props$, "data_fragment");
  return null;
};
exports.PropertiedView = PropertiedView;
function DefaultedView($props$) {
  var data_nil = $knot.platform.prop($props$, "data_nil", null);
  var data_true = $knot.platform.prop($props$, "data_true", true);
  var data_false = $knot.platform.prop($props$, "data_false", false);
  var data_positive_integer = $knot.platform.prop($props$,
                                                  "data_positive_integer",
                                                  123);
  var data_negative_integer = $knot.platform.prop($props$,
                                                  "data_negative_integer",
                                                  -(123));
  var data_positive_float = $knot.platform.prop($props$,
                                                "data_positive_float",
                                                456.78);
  var data_negative_float = $knot.platform.prop($props$,
                                                "data_negative_float",
                                                -(456.78));
  var data_string = $knot.platform.prop($props$, "data_string", "abc");
  var data_tag = $knot.platform.prop($props$,
                                     "data_tag",
                                     $knot.jsx.createTag("div"));
  var data_fragment = $knot.platform.prop($props$,
                                          "data_fragment",
                                          $knot.jsx.createFragment());
  return null;
};
exports.DefaultedView = DefaultedView;
function PartialView($props$) {
  var first = $knot.platform.prop($props$, "first");
  var second = $knot.platform.prop($props$, "second", false);
  return null;
};
exports.PartialView = PartialView;
function ExpressionView($props$) {
  var lhs = $knot.platform.prop($props$, "lhs");
  var rhs = $knot.platform.prop($props$, "rhs");
  return (lhs + rhs);
};
exports.ExpressionView = ExpressionView;
function ClosureView($props$) {
  var lhs = $knot.platform.prop($props$, "lhs");
  var rhs = $knot.platform.prop($props$, "rhs");
  var lhs_int = (lhs * 10);
  var rhs_float = (rhs / 13);
  return (lhs_int <= rhs_float);
};
exports.ClosureView = ClosureView;
function ImplicitChildren($props$) {
  return $knot.jsx.createTag("div",
                             null,
                             $knot.jsx.createTag("div", null, $.children));
};
exports.ImplicitChildren = ImplicitChildren;
function ExplicitChildren($props$) {
  var children = $knot.platform.prop($props$, "children");
  return $knot.jsx.createTag("div",
                             null,
                             $knot.jsx.createTag("div", null, children));
};
exports.ExplicitChildren = ExplicitChildren;
