open Core;
open Scope;

let analyze = scope =>
  fun
  | ConstDecl(name, expr) =>
    A_ConstDecl(name, Expression.analyze(scope, expr))
  | StateDecl(name, params, props) =>
    A_StateDecl(
      name,
      List.map(Property.analyze(scope), params),
      List.map(State.analyze_prop(scope), props),
    )
  | ViewDecl(name, super, mixins, params, exprs) =>
    A_ViewDecl(
      name,
      super,
      mixins,
      List.map(Property.analyze(scope), params),
      List.map(Expression.analyze(scope), exprs),
    )
  | FunctionDecl(name, params, exprs) =>
    A_FunctionDecl(
      name,
      List.map(Property.analyze(scope), params),
      List.map(Expression.analyze(scope), exprs),
    )
  | StyleDecl(name, params, rules) =>
    A_StyleDecl(
      name,
      List.map(Property.analyze(scope), params),
      List.map(Style.analyze_rule_set(scope), rules),
    );