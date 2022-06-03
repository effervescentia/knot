var $knot = require("@knot/runtime");
function explicit_nil_function() {
  return null;
};
exports.explicit_nil_function = explicit_nil_function;
function implicit_nil_function() { };
exports.implicit_nil_function = implicit_nil_function;
function no_args_function() {
  return null;
};
exports.no_args_function = no_args_function;
function many_args_function(data_nil,
                            data_true,
                            data_false,
                            data_positive_integer,
                            data_negative_integer,
                            data_positive_float,
                            data_negative_float,
                            data_string,
                            data_tag,
                            data_fragment) {
  return null;
};
exports.many_args_function = many_args_function;
function defaulted_function(data_nil,
                            data_true,
                            data_false,
                            data_positive_integer,
                            data_negative_integer,
                            data_positive_float,
                            data_negative_float,
                            data_string,
                            data_tag,
                            data_fragment) {
  data_nil = $knot.platform.arg(arguments, 0, null);
  data_true = $knot.platform.arg(arguments, 1, true);
  data_false = $knot.platform.arg(arguments, 2, false);
  data_positive_integer = $knot.platform.arg(arguments, 3, 123);
  data_negative_integer = $knot.platform.arg(arguments, 4, -(123));
  data_positive_float = $knot.platform.arg(arguments, 5, 456.78);
  data_negative_float = $knot.platform.arg(arguments, 6, -(456.78));
  data_string = $knot.platform.arg(arguments, 7, "abc");
  data_tag = $knot.platform.arg(arguments, 8, $knot.jsx.createTag("div"));
  data_fragment = $knot.platform.arg(arguments, 9, $knot.jsx.createFragment());
  return null;
};
exports.defaulted_function = defaulted_function;
function partial_function(first, second) {
  second = $knot.platform.arg(arguments, 1, false);
  return null;
};
exports.partial_function = partial_function;
function expression_function(lhs, rhs) {
  return (lhs + rhs);
};
exports.expression_function = expression_function;
function closure_function(lhs, rhs) {
  var lhs_int = (lhs * 10);
  var rhs_float = (rhs / 13);
  return (lhs_int <= rhs_float);
};
exports.closure_function = closure_function;
