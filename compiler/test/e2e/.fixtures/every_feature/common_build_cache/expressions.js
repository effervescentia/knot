var $knot = require("@knot/runtime");
var JSX = $knot.stdlib.JSX;
var $import$_$functions = require("./functions");
var no_args_function = $import$_$functions.no_args_function;
var expression_function = $import$_$functions.expression_function;
$import$_$functions = null;
var not_expr = !(true);
exports.not_expr = not_expr;
var negated_expr = -(-(123));
exports.negated_expr = negated_expr;
var absolute_expr = +(-(123));
exports.absolute_expr = absolute_expr;
var and_expr = (true && false);
exports.and_expr = and_expr;
var or_expr = (false || true);
exports.or_expr = or_expr;
var add_expr = (123 + 456.78);
exports.add_expr = add_expr;
var subtract_expr = (123 - 456.78);
exports.subtract_expr = subtract_expr;
var multiply_expr = (123 + 456.78);
exports.multiply_expr = multiply_expr;
var divide_expr = (123 / 456.78);
exports.divide_expr = divide_expr;
var exponent_expr = Math.pow(123, 456.78);
exports.exponent_expr = exponent_expr;
var less_than_expr = (123 < 456.78);
exports.less_than_expr = less_than_expr;
var less_than_or_equal_expr = (123 <= 456.78);
exports.less_than_or_equal_expr = less_than_or_equal_expr;
var greater_than_expr = (123 < 456.78);
exports.greater_than_expr = greater_than_expr;
var greater_than_or_equal_expr = (123 >= 456.78);
exports.greater_than_or_equal_expr = greater_than_or_equal_expr;
var equality_expr = ("abc" === "def");
exports.equality_expr = equality_expr;
var inequality_expr = ("abc" !== "abc");
exports.inequality_expr = inequality_expr;
var identifier_expr = (add_expr * subtract_expr);
exports.identifier_expr = identifier_expr;
var group_expr = (10 - (2 + 100));
exports.group_expr = group_expr;
var empty_closure_expr = null;
exports.empty_closure_expr = empty_closure_expr;
var closure_stmts_expr = (function () {
                            var x = (100 / 200);
                            return (x + 5);
                          })();
exports.closure_stmts_expr = closure_stmts_expr;
var dot_access_expr = JSX.render;
exports.dot_access_expr = dot_access_expr;
var empty_function_call_expr = no_args_function();
exports.empty_function_call_expr = empty_function_call_expr;
var function_call_with_args_expr = expression_function(333, 444);
exports.function_call_with_args_expr = function_call_with_args_expr;
var tag_with_expr_attributes = $knot.jsx.createTag("div",
                                                   {
                                                     closure_stmts_attr: (
                                                     function () {
                                                       var y = (true && false);
                                                       return (y || false);
                                                     })(),
                                                     empty_closure_attr: null,
                                                     group_attr: ("abc"),
                                                     binary_op_attr: (true && false),
                                                     unary_op_attr: +(-(+(-(123)))),
                                                     id_attr: not_expr
                                                   });
exports.tag_with_expr_attributes = tag_with_expr_attributes;
