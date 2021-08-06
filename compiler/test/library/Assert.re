open Kore;

include Test.Assert;

module Compare = {
  open Alcotest;

  include Test.Assert.Compare;

  let input =
    testable(
      pp =>
        (
          x =>
            Print.fmt(
              "%s@%s",
              [x |> Input.value] |> String.of_uchars,
              x |> Input.point |> Point.to_string,
            )
        )
        % Format.pp_print_string(pp),
      (==),
    );

  let point =
    testable(pp => Point.to_string % Format.pp_print_string(pp), (==));

  let range =
    testable(pp => Range.to_string % Format.pp_print_string(pp), (==));

  let graph =
    testable(
      pp =>
        Graph.to_string(Functional.identity) % Format.pp_print_string(pp),
      (==),
    );

  let namespace =
    testable(
      pp => Reference.Namespace.to_string % Format.pp_print_string(pp),
      (==),
    );

  let module_table =
    testable(
      pp => ModuleTable.to_string % Format.pp_print_string(pp),
      ModuleTable.(
        (l, r) =>
          Hashtbl.compare(
            ~compare=
              (x, y) =>
                x.ast == y.ast
                && x.raw == y.raw
                && Hashtbl.compare(x.exports, y.exports),
            l,
            r,
          )
      ),
    );
};

let char = Alcotest.(check(Compare.input, "input matches"));

let point = Alcotest.(check(Compare.point, "point matches"));

let range = Alcotest.(check(Compare.range, "range matches"));

let graph = Alcotest.(check(Compare.graph, "graph matches"));

let namespace = Alcotest.(check(Compare.namespace, "namespace matches"));
let list_namespace =
  Alcotest.(check(list(Compare.namespace), "namespace list matches"));

let module_table =
  Alcotest.check(Compare.module_table, "module table matches");

let compile_errors =
  Alcotest.(
    check(
      testable(pp => Util.print_errs % Format.pp_print_string(pp), (==)),
      "compile error matches",
    )
  );
