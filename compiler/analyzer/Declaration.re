open Core;
open Scope;

let analyze = (scope, name) =>
  fun
  | ConstDecl(expr) => Expression.analyze(scope, expr)
  | StateDecl(params, props) => {
      List.iter(Property.analyze(Expression.analyze, scope), params);
      List.iter(State.analyze_prop(scope), props);
    }
  | ViewDecl(super, mixins, params, exprs) => {
      let nested_scope =
        scope.nest(~label=Printf.sprintf("view(%s)", name), ());

      List.iter(Property.analyze(Expression.analyze, nested_scope), params);
      List.iter(Expression.analyze(nested_scope), exprs);
    }
  | FunctionDecl(params, exprs) => {
      let nested_scope =
        scope.nest(~label=Printf.sprintf("function(%s)", name), ());

      List.iter(
        res => {
          Property.analyze(
            ~resolve=false,
            Expression.analyze,
            nested_scope,
            res,
          );
          Resolver.of_parameter(res) |> nested_scope.resolve;
        },
        params,
      );
      List.iter(Expression.analyze(nested_scope), exprs);
    }
  | StyleDecl(params, rules) => {
      List.iter(Property.analyze(Expression.analyze, scope), params);
      List.iter(Style.analyze_rule_set(scope), rules);
    };
