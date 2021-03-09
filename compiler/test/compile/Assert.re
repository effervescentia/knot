open Kore;

include TestResolve.Assert;

let module_table =
  Alcotest.(
    check(
      testable(
        pp => Compile.ModuleTable.to_string % Format.pp_print_string(pp),
        Compile.ModuleTable.(
          (l, r) =>
            Hashtbl.compare(
              ~compare=
                (x, y) =>
                  x.ast == y.ast && Hashtbl.compare(x.types, y.types),
              l,
              r,
            )
        ),
      ),
      "module table matches",
    )
  );

let program =
  Alcotest.(
    check(
      testable(
        pp => Grammar.Formatter.format % Format.pp_print_string(pp),
        (==),
      ),
      "program matches",
    )
  );
