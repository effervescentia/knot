open Globals;
open Scope;

let analyze = (scope, name) =>
  fun
  | ConstDecl(expr) => Expression.analyze(scope, expr)

  | FunctionDecl(params, exprs) => {
      let nested_scope =
        scope.nest(~label=Printf.sprintf("function(%s)", name), ());

      Function.analyze(Expression.analyze, nested_scope, params, exprs);
    }

  | ViewDecl(super, mixins, params, exprs) => {
      let nested_scope =
        scope.nest(
          ~label=Printf.sprintf("view(%s)", name),
          ~sidecar=Hashtbl.create(List.length(mixins)),
          (),
        );

      List.iter(Resolver.of_mixin % nested_scope.resolve, mixins);

      Function.analyze(Expression.analyze, nested_scope, params, exprs);
    }

  | StateDecl(params, props) => {
      let nested_scope =
        scope.nest(
          ~label=Printf.sprintf("state(%s)", name),
          ~sidecar=Hashtbl.create(List.length(props)),
          (),
        );

      List.iter(
        Property.analyze_param(Expression.analyze, nested_scope),
        params,
      );
      List.iter(
        ((name, prop)) => State.analyze_prop(nested_scope, name, prop),
        props,
      );
    }

  | StyleDecl(params, rule_sets) => {
      let nested_scope =
        scope.nest(~label=Printf.sprintf("style(%s)", name), ());

      List.iter(
        Property.analyze_param(Expression.analyze, nested_scope),
        params,
      );
      List.iter(Style.analyze_rule_set(nested_scope), rule_sets);
    };
