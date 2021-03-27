open Kore;

include TestResolve.Assert;

let compiler =
  Alcotest.(
    check(
      testable(
        (pp, _) => "" |> Format.pp_print_string(pp),
        Compile.Compiler.(
          (l, r) => {
            Alcotest.check(
              Compare.module_table,
              "module_table matches",
              l.modules,
              r.modules,
            );

            l.graph.imports == r.graph.imports
            && l.resolver.root_dir == r.resolver.root_dir
            && l.config == r.config
            && l.errors == r.errors
            && l.throw === r.throw;
          }
        ),
      ),
      "compiler matches",
    )
  );
