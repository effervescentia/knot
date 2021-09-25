open Kore;

include Test.Assert;

module Compare = {
  open Alcotest;

  include Test.Assert.Compare;

  let trio =
    Alcotest.(
      (a, b, c) => {
        let eq = ((a1, b1, c1), (a2, b2, c2)) =>
          equal(a, a1, a2) && equal(b, b1, b2) && equal(c, c1, c2);
        testable(Tuple.pp3(pp(a), pp(b), pp(c)), eq);
      }
    );

  let input =
    testable(
      pp =>
        (
          x =>
            Print.fmt(
              "%s@%s",
              [Input.get_value(x)] |> String.of_uchars,
              x |> Input.get_point |> Point.to_string,
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

  let raw_node = (value_to_string: 'a => string) =>
    testable(
      (pp, node) =>
        Print.fmt(
          "%s @ %s",
          value_to_string(Node.Raw.get_value(node)),
          Range.to_string(Node.Raw.get_range(node)),
        )
        |> Format.pp_print_string(pp),
      (==),
    );

  let node = (value_to_string: 'a => string) =>
    testable(
      (pp, node) =>
        Print.fmt(
          "%s (%s) @ %s",
          value_to_string(Node.get_value(node)),
          Type.to_string(Node.get_type(node)),
          Range.to_string(Node.get_range(node)),
        )
        |> Format.pp_print_string(pp),
      (==),
    );

  let type_ =
    testable(pp => Type.to_string % Format.pp_print_string(pp), (==));

  let target =
    testable(pp => Target.to_string % Format.pp_print_string(pp), (==));

  exception LazyStreamLengthMismatch;

  let lazy_stream = (value_to_string: 'a => string) =>
    testable(
      pp =>
        LazyStream.to_list
        % Print.many(value_to_string)
        % Format.pp_print_string(pp),
      (expected, actual) => {
        let rec loop =
          LazyStream.(
            fun
            | (Cons(l, next_l), Cons(r, next_r)) => {
                let (ls, rs) =
                  loop((Lazy.force(next_l), Lazy.force(next_r)));

                ([l, ...ls], [r, ...rs]);
              }
            | (Nil, Nil) => ([], [])
            | _ => raise(LazyStreamLengthMismatch)
          );

        switch (loop((expected, actual))) {
        | (expected_values, actual_values) => expected_values == actual_values
        | exception LazyStreamLengthMismatch => false
        };
      },
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

let int_trio =
  Alcotest.(check(Compare.trio(int, int, int), "int trio matches"));

let input = Alcotest.(check(Compare.input, "input matches"));

let point = Alcotest.(check(Compare.point, "point matches"));

let range = Alcotest.(check(Compare.range, "range matches"));

let graph = Alcotest.(check(Compare.graph, "graph matches"));

let raw_node = value_to_string =>
  Alcotest.(check(Compare.raw_node(value_to_string), "raw node matches"));

let node = value_to_string =>
  Alcotest.(check(Compare.node(value_to_string), "node matches"));

let type_ = Alcotest.(check(Compare.type_, "type matches"));

let target = Alcotest.(check(Compare.target, "target matches"));
let opt_target = Alcotest.(check(option(Compare.target), "target matches"));

let lazy_stream = value_to_string =>
  Alcotest.(
    check(Compare.lazy_stream(value_to_string), "lazy stream matches")
  );

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
