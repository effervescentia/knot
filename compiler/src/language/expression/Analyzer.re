open Kore;

let _get_identifier =
  Interface.(
    fun
    | Identifier(id) => Some(id)
    | _ => None
  );

let rec analyze:
  AST.Framework.Interface.analyze_t(
    'ast,
    Interface.t(unit),
    Interface.t(AST.Type.t),
  ) =
  (scope, (statement, _) as node) => {
    let bind = (analyze, to_expr, arg, value) =>
      node
      |> Node.map(_ => value)
      |> analyze(arg, scope)
      |> Tuple.map_fst2(to_expr);

    statement
    |> Interface.fold(
         ~primitive=bind(Primitive.analyze, Interface.of_primitive, ()),
         ~identifier=bind(Identifier.analyze, Interface.of_identifier, ()),
         ~unary_op=
           bind(
             UnaryOperator.analyze,
             ((op, expr)) => Interface.of_unary_op(op, expr),
             analyze,
           ),
         ~binary_op=
           bind(
             BinaryOperator.analyze,
             ((op, lhs, rhs)) => (lhs, rhs) |> Interface.of_binary_op(op),
             analyze,
           ),
         ~group=bind(Group.analyze, Interface.of_group, analyze),
         ~closure=bind(Closure.analyze, Interface.of_closure, analyze),
         ~dot_access=
           bind(DotAccess.analyze, Interface.of_dot_access, analyze),
         ~bind_style=
           bind(
             BindStyle.analyze,
             ((kind, lhs, rhs)) =>
               Interface.of_bind_style(kind, (lhs, rhs)),
             (analyze, (_get_identifier, Interface.of_identifier)),
           ),
         ~function_call=
           bind(FunctionCall.analyze, Interface.of_function_call, analyze),
         ~style=bind(Style.analyze, Interface.of_style, analyze),
         ~ksx=bind(KSX.analyze, Interface.of_ksx, analyze),
       );
  };
