open Core;
open Scope;

let analyze = (scope, name) =>
  fun
  | ConstDecl(expr) => Expression.analyze(scope, expr)

  | FunctionDecl(params, exprs) => {
      let nested_scope =
        scope.nest(~label=Printf.sprintf("function(%s)", name), ());

      List.iter(
        Property.analyze_param(Expression.analyze, nested_scope),
        params,
      );
      List.iter(Function.analyze_scoped_expr(nested_scope), exprs);
    }

  | ViewDecl(super, mixins, params, exprs) => {
      let nested_scope =
        scope.nest(
          ~label=Printf.sprintf("view(%s)", name),
          ~sidecar=Hashtbl.create(List.length(mixins)),
          (),
        );

      List.iter(Resolver.of_mixin % nested_scope.resolve, mixins);

      List.iter(
        Property.analyze_param(Expression.analyze, nested_scope),
        params,
      );
      List.iter(Function.analyze_scoped_expr(nested_scope), exprs);
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

  | _ => raise(NotImplemented);
