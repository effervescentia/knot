open Core;

let analyze_rule = (scope, (key, value)) =>
  (
    Reference.analyze(Expression.analyze, scope, key),
    Reference.analyze(Expression.analyze, scope, value),
  )
  |> await_ctx;

let analyze_rule_set = (scope, (key, rules): Knot.AST.style_rule_set) =>
  (await_ctx(key), List.map(analyze_rule(scope), rules)) |> await_ctx;