open Kore;

module T = AST.Type;
module Primitive = KPrimitive.Plugin;
module Expression = KExpression.Plugin;

module CommonUtil = {
  let as_untyped = (~range=Range.zero, x) => Node.raw(x, range);
  let as_typed = (~range=Range.zero, type_, x) =>
    Node.typed(x, type_, range);
};

module RawUtil = {
  include CommonUtil;

  /* typecasting utilities */

  let as_node = x => as_typed((), x);
  let as_unknown = x => as_typed(`Unknown, x);
  let as_nil = x => as_typed(T.Nil, x);
  let as_bool = x => as_typed(T.Boolean, x);
  let as_int = x => as_typed(T.Integer, x);
  let as_float = x => as_typed(T.Float, x);
  let as_string = x => as_typed(T.String, x);
  let as_element = x => as_typed(T.Element, x);
  let as_style = x => as_typed(T.Style, x);
  let as_struct = (props, x) => as_typed(T.Object(props), x);
  let as_function = (args, res, x) => as_typed(T.Function(args, res), x);
  let as_decorator = (args, target, x) =>
    as_typed(T.Decorator(args, target), x);

  /* primitive factories */

  let nil_prim: Node.t(Expression.t(unit), unit) =
    Primitive.nil |> Expression.of_primitive |> as_node;
  let bool_prim = x =>
    Primitive.of_boolean(x) |> Expression.of_primitive |> as_node;
  let int_prim = x =>
    Int64.of_int(x)
    |> Primitive.of_integer
    |> Expression.of_primitive
    |> as_node;
  let float_prim = x =>
    Primitive.of_float(x) |> Expression.of_primitive |> as_node;
  let string_prim = x =>
    Primitive.of_string(x) |> Expression.of_primitive |> as_node;

  /* jsx factories */

  let ksx_node = x =>
    x |> KSX.Interface.of_element_tag |> KSX.Interface.Child.of_node;
  let ksx_tag = x => x |> KSX.Interface.of_element_tag |> Expression.of_ksx;
};

module ResultUtil = {
  include CommonUtil;

  /* typecasting utilities */

  let as_invalid = (inv, x) => as_typed(T.Invalid(inv), x);
  let as_nil = x => as_typed(T.Valid(Nil), x);
  let as_bool = x => as_typed(T.Valid(Boolean), x);
  let as_int = x => as_typed(T.Valid(Integer), x);
  let as_float = x => as_typed(T.Valid(Float), x);
  let as_string = x => as_typed(T.Valid(String), x);
  let as_element = x => as_typed(T.Valid(Element), x);
  let as_style = x => as_typed(T.Valid(Style), x);
  let as_enum = (variants, x) =>
    as_typed(T.Valid(Enumerated(variants)), x);
  let as_struct = (props, x) => as_typed(T.Valid(Object(props)), x);
  let as_function = (args, res, x) =>
    as_typed(T.Valid(Function(args, res)), x);
  let as_view = (props, res, x) => as_typed(T.Valid(View(props, res)), x);
  let as_decorator = (args, target, x) =>
    as_typed(T.Valid(Decorator(args, target)), x);

  /* primitive factories */

  let nil_prim: Node.t(Expression.t(T.t), T.t) =
    Primitive.nil |> Expression.of_primitive |> as_nil;
  let bool_prim = x =>
    Primitive.of_boolean(x) |> Expression.of_primitive |> as_bool;
  let int_prim = x =>
    Int64.of_int(x)
    |> Primitive.of_integer
    |> Expression.of_primitive
    |> as_int;
  let float_prim = x =>
    Primitive.of_float(x) |> Expression.of_primitive |> as_float;
  let string_prim = x =>
    Primitive.of_string(x) |> Expression.of_primitive |> as_string;

  /* jsx factories */

  let ksx_node = x =>
    x |> KSX.Interface.of_element_tag |> KSX.Interface.Child.of_node;
  let ksx_tag = x => x |> KSX.Interface.of_element_tag |> Expression.of_ksx;
};
