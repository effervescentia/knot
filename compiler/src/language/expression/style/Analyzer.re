open Knot.Kore;
open AST;

let analyze_rule_name =
    (scope: Scope.t('ast), (rule_name, _) as node: Node.t(string, unit)) => {
  let resolve_err = err => {
    err |> Scope.report_type_err(scope, Node.get_range(node));
    Type.Invalid(NotInferrable);
  };

  let type_ =
    scope
    |> Scope.lookup(rule_name)
    |> (
      fun
      | Some(Ok(Valid(Function([Valid(_) as t], Valid(Nil))))) => t
      | Some(Error(err)) => resolve_err(err)
      | _ => Type.UnknownStyleRule(rule_name) |> resolve_err
    );

  node |> Node.add_type(type_);
};

let analyze_rule =
    (
      analyze_name,
      analyze_expression,
      ((name, expression), _) as node:
        Interface.StyleRule.node_t('expr, unit),
    ) => {
  let (name', name_type) = analyze_name(name);
  let (expression', expression_type) =
    expression |> Node.analyzer(analyze_expression);

  (
    node |> Node.map(_ => (name', expression')),
    (name_type, expression_type),
  );
};

let analyze: Interface.Plugin.analyze_t('ast, 'raw_expr, 'result_expr) =
  (analyze_expression, scope, (rules, _) as node) => {
    let range = Node.get_range(node);
    let rule_scope =
      scope |> Scope.create_augmented(~prefix="", StyleRule, range);
    let expression_scope =
      scope |> Scope.create_augmented(StyleExpression, range);
    let analyze_expression =
      Node.analyzer(analyze_expression(expression_scope));

    let rules' =
      rules
      |> List.map(
           Node.map(
             Tuple.map_each2(
               analyze_rule_name(rule_scope),
               analyze_expression % fst,
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

    (rules', Type.Valid(Style));
  };
