open Core;
open Scope;

let rec analyze = (analyze_expr, scope, refr) => {
  fst(refr)
  |> (
    fun
    | Variable(name)
    | SidecarVariable(name) => ()

    | DotAccess(lhs, rhs) => analyze(analyze_expr, scope, lhs)
    | Execution(target, args) => {
        switch (fst(target)) {
        | Variable(_) => ()
        | _ => analyze(analyze_expr, scope, target)
        };

        List.iter(analyze_expr(scope), args);
      }
  );

  Resolver.of_reference(refr) |> scope.resolve;
};
