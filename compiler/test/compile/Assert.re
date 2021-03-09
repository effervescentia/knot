open Kore;

include TestResolve.Assert;

let _compare_module_table =
  Compile.ModuleTable.(
    (l, r) =>
      Hashtbl.compare(
        ~compare=
          (x, y) => x.ast == y.ast && Hashtbl.compare(x.types, y.types),
        l,
        r,
      )
  );

let module_table =
  Alcotest.(
    check(
      testable(
        pp => Compile.ModuleTable.to_string % Format.pp_print_string(pp),
        _compare_module_table,
      ),
      "module table matches",
    )
  );

let compiler =
  Alcotest.(
    check(
      testable(
        (pp, _) => "" |> Format.pp_print_string(pp),
        Compile.Compiler.(
          (l, r) =>
            _compare_module_table(l.modules, r.modules)
            && l.graph.imports == r.graph.imports
            && l.resolver.root_dir == r.resolver.root_dir
            && l.config == r.config
            && l.errors == r.errors
            && l.throw === r.throw
        ),
      ),
      "compiler matches",
    )
  );
