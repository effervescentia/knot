open Core;

let analyze_rule = (scope, (key, value)) => (
  Reference.analyze(Expression.analyze, scope, key) |> wrap,
  Reference.analyze(Expression.analyze, scope, value) |> wrap,
);

let analyze_rule_set = (scope, (key, rules): Knot.AST.style_rule_set) => (
  wrap(key),
  analyze_list(analyze_rule(scope), rules),
);