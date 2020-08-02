open Globals;

let analyze_rule = (scope, (key, value)) => {
  Reference.analyze(Expression.analyze, scope, key);
  Reference.analyze(Expression.analyze, scope, value);
};

let analyze_rule_set = (scope, (key, rules): Knot.AST.style_rule_set) =>
  List.iter(analyze_rule(scope), rules);
