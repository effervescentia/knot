open Kore;

include Test.Assert;

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

let _check_m_id =
  Alcotest.(
    testable(
      pp => Resolve.Kore.print_m_id % Format.pp_print_string(pp),
      (==),
    )
  );

let m_id = Alcotest.(check(_check_m_id, "module id matches"));
let list_m_id =
  Alcotest.(check(list(_check_m_id), "module id list matches"));

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
