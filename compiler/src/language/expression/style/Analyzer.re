open Knot.Kore;

module Scope = AST.Scope;
module Type = AST.Type;

let validate_style_rule =
    (name): (((Type.t, Type.t)) => option(Type.error_t)) =>
  fun
  /* assume this has been reported already and ignore */
  | (_, Invalid(_))
  | (Invalid(_), _) => None

  | (arg_type, value_type) when arg_type == value_type => None

  /* special override for raw string styles */
  | (_, Valid(`String)) => None

  | (expected_type, actual_type) =>
    Some(InvalidStyleRule(name, expected_type, actual_type));

let analyze_style_rule = (scope: Scope.t, raw_rule: Node.t(string, unit)) => {
  let key = fst(raw_rule);
  let type_ =
    scope
    |> Scope.lookup(key)
    |> (
      fun
      | Some(Valid(`Function([Valid(_) as t], Valid(`Nil)))) => t
      | _ => {
          let err = Type.UnknownStyleRule(key);

          err |> Scope.report_type_err(scope, Node.get_range(raw_rule));

          Type.Invalid(NotInferrable);
        }
    );

  raw_rule |> Node.add_type(type_);
};

let analyze_style:
  (
    Scope.t,
    (Scope.t, AST.Raw.expression_t) => AST.Result.expression_t,
    list(AST.Raw.style_rule_t),
    Range.t
  ) =>
  (list(AST.Result.style_rule_t), Type.t) =
  (scope, analyze_expression, rules, range) => {
    let rule_scope = scope |> Scope.create_child(range);
    let expression_scope = scope |> Scope.create_child(range);
    Scope.inject_plugin_types(~prefix="", StyleRule, rule_scope);
    Scope.inject_plugin_types(StyleExpression, expression_scope);

    let rules' =
      rules
      |> List.map(
           Node.map(
             Tuple.map_each2(
               analyze_style_rule(rule_scope),
               analyze_expression(expression_scope),
             ),
           ),
         );

    rules'
    |> List.map(
         fst
         % (
           ((key, value)) =>
             validate_style_rule(
               fst(key),
               (Node.get_type(key), Node.get_type(value)),
             )
         ),
       )
    |> List.iter(
         Option.iter(Scope.report_type_err(expression_scope, range)),
       );

    (rules', Type.Valid(`Style));
  };
