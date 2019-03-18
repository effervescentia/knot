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
      List.iter(Expression.analyze(nested_scope), exprs);

      nested_scope.validate();
    }

  | ViewDecl(super, mixins, params, exprs) => {
      let nested_scope =
        scope.nest(~label=Printf.sprintf("view(%s)", name), ());

      List.iter(
        Property.analyze_param(Expression.analyze, nested_scope),
        params,
      );
      List.iter(Expression.analyze(nested_scope), exprs);

      nested_scope.validate();
    }

  | _ => raise(NotImplemented);
/*

 | StateDecl(params, props) => {
     let nested_scope =
       scope.nest(~label=Printf.sprintf("state(%s)", name), ());

     List.iter(
       Property.analyze_param(Expression.analyze, nested_scope),
       params,
     );
     List.iter(State.analyze_prop(nested_scope), props);
   }

 | StyleDecl(params, rules) => {
     List.iter(Property.analyze_param(Expression.analyze, scope), params);
     List.iter(Style.analyze_rule_set(scope), rules);
   }; */
