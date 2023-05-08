import $knot from "@knot/runtime";
function ExplicitNilView($props$) {
  return null;
};
export { ExplicitNilView };
function ImplicitNilView($props$) { };
export { ImplicitNilView };
function NoPropertiesView($props$) {
  return null;
};
export { NoPropertiesView };
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
export { PropertiedView };
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
export { DefaultedView };
function PartialView($props$) {
  var first = $knot.platform.prop($props$, "first");
  var second = $knot.platform.prop($props$, "second", false);
  return null;
};
export { PartialView };
function ExpressionView($props$) {
  var lhs = $knot.platform.prop($props$, "lhs");
  var rhs = $knot.platform.prop($props$, "rhs");
  return (lhs + rhs);
};
export { ExpressionView };
function ClosureView($props$) {
  var lhs = $knot.platform.prop($props$, "lhs");
  var rhs = $knot.platform.prop($props$, "rhs");
  var lhs_int = (lhs * 10);
  var rhs_float = (rhs / 13);
  return (lhs_int <= rhs_float);
};
export { ClosureView };
function ImplicitChildren($props$) {
  return $knot.jsx.createTag("div",
                             null,
                             $knot.jsx.createTag("div", null, $.children));
};
export { ImplicitChildren };
function ExplicitChildren($props$) {
  var children = $knot.platform.prop($props$, "children");
  return $knot.jsx.createTag("div",
                             null,
                             $knot.jsx.createTag("div", null, children));
};
export { ExplicitChildren };
