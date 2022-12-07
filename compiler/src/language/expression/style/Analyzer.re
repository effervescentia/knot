open Knot.Kore;
open AST;

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

let analyze:
  (
    Scope.t,
    (Scope.t, Raw.expression_t) => Result.expression_t,
    list(Raw.style_rule_t),
    Range.t
  ) =>
  (list(Result.style_rule_t), Type.t) =
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
             Validator.validate_style_rule(
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
