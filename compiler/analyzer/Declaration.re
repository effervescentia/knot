open Core;
open Scope;

let analyze = scope =>
  fun
  | ConstDecl(name, expr) =>
    A_ConstDecl(name, Expression.analyze(scope, expr))
  | StateDecl(name, params, props) =>
    A_StateDecl(
      name,
      List.map(Property.analyze(Expression.analyze, scope), params),
      List.map(State.analyze_prop(scope), props),
    )
  | ViewDecl(name, super, mixins, params, exprs) => {
      let nested_scope =
        scope.nest(~label=Printf.sprintf("view(%s)", name), ());

      A_ViewDecl(
        name,
        super,
        mixins,
        List.map(Property.analyze(Expression.analyze, nested_scope), params),
        List.map(Expression.analyze(nested_scope), exprs),
      );
    }
  | FunctionDecl(name, params, exprs) => {
      let nested_scope =
        scope.nest(~label=Printf.sprintf("function(%s)", name), ());
      let ctxl_params =
        List.map(
          Property.analyze(~resolve=false, Expression.analyze, nested_scope)
          % (
            res => {
              Resolver.of_parameter(res) |> nested_scope.resolve;

              res;
            }
          ),
          params,
        );

      A_FunctionDecl(
        name,
        ctxl_params,
        List.map(Expression.analyze(nested_scope), exprs),
      );
    }
  | StyleDecl(name, params, rules) =>
    A_StyleDecl(
      name,
      List.map(Property.analyze(Expression.analyze, scope), params),
      List.map(Style.analyze_rule_set(scope), rules),
    );
