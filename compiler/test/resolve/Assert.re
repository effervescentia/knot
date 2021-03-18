open Kore;

include Test.Assert;

let import_graph =
  Alcotest.(
    check(
      testable(
        pp => Resolve.ImportGraph.to_string % Format.pp_print_string(pp),
        (l, r) => l.imports == r.imports && l.get_imports === r.get_imports,
      ),
      "import graph matches",
    )
  );

let check_namespace =
  Alcotest.(
    testable(
      pp => AST.string_of_namespace % Format.pp_print_string(pp),
      (==),
    )
  );

let namespace = Alcotest.(check(check_namespace, "namespace matches"));
let list_namespace =
  Alcotest.(check(list(check_namespace), "namespace list matches"));

let module_ =
  Alcotest.(
    check(
      testable(
        pp =>
          Resolve.Module.(
            fun
            | Raw(s) => Print.fmt("raw: %s", s)
            | File({full, relative}) =>
              Print.fmt("full: %s, relative: %s", full, relative)
          )
          % Format.pp_print_string(pp),
        (==),
      ),
      "module matches",
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

let resolver =
  Alcotest.(
    check(
      testable(
        Resolve.Resolver.(
          (pp, resolver) =>
            Print.fmt(
              "cache: %s, root_dir: %s",
              resolver.cache,
              resolver.root_dir,
            )
            |> Format.pp_print_string(pp)
        ),
        (==),
      ),
      "resolver matches",
    )
  );