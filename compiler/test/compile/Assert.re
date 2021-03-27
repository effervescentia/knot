open Kore;

include TestResolve.Assert;

module Compare = {
  open Alcotest;

  include TestResolve.Assert.Compare;

  let module_table =
    testable(
      pp => Compile.Debug.print_module_table % Format.pp_print_string(pp),
      Compile.ModuleTable.(
        (l, r) =>
          Hashtbl.compare(
            ~compare=
              (x, y) => x.ast == y.ast && Hashtbl.compare(x.types, y.types),
            l,
            r,
          )
      ),
    );
};

let module_table =
  Alcotest.check(Compare.module_table, "module table matches");

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
