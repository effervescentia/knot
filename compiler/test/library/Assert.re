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
              x |> Input.cursor |> Cursor.to_string,
            )
        )
        % Format.pp_print_string(pp),
      (==),
    );

  let cursor =
    testable(pp => Cursor.to_string % Format.pp_print_string(pp), (==));

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
};

let char = Alcotest.(check(Compare.input, "input matches"));

let cursor = Alcotest.(check(Compare.cursor, "cursor matches"));

let graph = Alcotest.(check(Compare.graph, "graph matches"));

let namespace = Alcotest.(check(Compare.namespace, "namespace matches"));
let list_namespace =
  Alcotest.(check(list(Compare.namespace), "namespace list matches"));
