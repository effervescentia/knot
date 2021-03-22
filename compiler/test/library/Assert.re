open Kore;

include Test.Assert;

let char =
  Alcotest.(
    check(
      testable(
        pp =>
          (
            x =>
              Print.fmt(
                "%s@%s",
                [x |> Input.value] |> String.of_uchars,
                x |> Input.context |> Cursor.to_string,
              )
          )
          % Format.pp_print_string(pp),
        (==),
      ),
      "cursor matches",
    )
  );

let cursor =
  Alcotest.(
    check(
      testable(pp => Cursor.to_string % Format.pp_print_string(pp), (==)),
      "cursor matches",
    )
  );

let graph =
  Alcotest.(
    check(
      testable(
        pp =>
          Graph.to_string(Functional.identity) % Format.pp_print_string(pp),
        (==),
      ),
      "graph matches",
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
