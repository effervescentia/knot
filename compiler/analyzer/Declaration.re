open Core;
open Scope;

let analyze = scope =>
  fun
  | ConstDecl(name, expr) =>
    A_ConstDecl(name, Expression.analyze(scope, expr) |> wrap)
  | StateDecl(name, params, props) => {
      scope.add(name, {type_: State, locality: Module});

      A_StateDecl(
        name,
        analyze_list(Property.analyze(scope), params),
        analyze_list(State.analyze_prop(scope), props),
      );
    }
  | ViewDecl(name, super, mixins, params, exprs) =>
    A_ViewDecl(
      name,
      super,
      mixins,
      analyze_list(Property.analyze(scope), params),
      analyze_list(Expression.analyze(scope), exprs),
    )
  | FunctionDecl(name, params, exprs) =>
    A_FunctionDecl(
      name,
      analyze_list(Property.analyze(scope), params),
      analyze_list(Expression.analyze(scope), exprs),
    )
  | StyleDecl(name, params, rules) =>
    A_StyleDecl(
      name,
      analyze_list(Property.analyze(scope), params),
      analyze_list(Style.analyze_rule_set(scope), rules),
    );