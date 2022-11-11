import $knot from "@knot/runtime";
var JSX = $knot.stdlib.JSX;
import { no_args_function, expression_function } from "./functions";
var not_expr = !(true);
export { not_expr };
var negated_expr = -(-(123));
export { negated_expr };
var absolute_expr = Math.abs(-(123));
export { absolute_expr };
var and_expr = (true && false);
export { and_expr };
var or_expr = (false || true);
export { or_expr };
var add_expr = (123 + 456.78);
export { add_expr };
var subtract_expr = (123 - 456.78);
export { subtract_expr };
var multiply_expr = (123 + 456.78);
export { multiply_expr };
var divide_expr = (123 / 456.78);
export { divide_expr };
var exponent_expr = Math.pow(123, 456.78);
export { exponent_expr };
var less_than_expr = (123 < 456.78);
export { less_than_expr };
var less_than_or_equal_expr = (123 <= 456.78);
export { less_than_or_equal_expr };
var greater_than_expr = (123 < 456.78);
export { greater_than_expr };
var greater_than_or_equal_expr = (123 >= 456.78);
export { greater_than_or_equal_expr };
var equality_expr = ("abc" === "def");
export { equality_expr };
var inequality_expr = ("abc" !== "abc");
export { inequality_expr };
var identifier_expr = (add_expr * subtract_expr);
export { identifier_expr };
var group_expr = (10 - (2 + 100));
export { group_expr };
var empty_closure_expr = null;
export { empty_closure_expr };
var closure_stmts_expr = (function () {
                            var x = (100 / 200);
                            return (x + 5);
                          })();
export { closure_stmts_expr };
var dot_access_expr = JSX.render;
export { dot_access_expr };
var empty_function_call_expr = no_args_function();
export { empty_function_call_expr };
var function_call_with_args_expr = expression_function(333, 444);
export { function_call_with_args_expr };
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
                                                     unary_op_attr: Math.abs(
                                                     -(Math.abs(-(123)))),
                                                     id_attr: not_expr
                                                   });
export { tag_with_expr_attributes };
