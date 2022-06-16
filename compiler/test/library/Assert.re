open Kore;

include Test.Assert;

module Compare = {
  open Alcotest;

  include Test.Assert.Compare;

  let trio =
    Alcotest.(
      (a, b, c) =>
        testable(
          Tuple.pp3(pp(a), pp(b), pp(c)), ((a1, b1, c1), (a2, b2, c2)) =>
          equal(a, a1, a2) && equal(b, b1, b2) && equal(c, c1, c2)
        )
    );

  let input = testable(Input.pp, (==));

  let point = testable(Point.pp, (==));

  let range = testable(Range.pp, (==));

  let graph = pp_value => testable(Graph.pp(pp_value), (==));

  let untyped_node = (pp_value: Fmt.t('a)) =>
    testable(Node.pp_untyped(pp_value), (==));

  let typed_node = (pp_value: Fmt.t('a), pp_type: Fmt.t('b)) =>
    testable(Node.pp(pp_value, pp_type), (==));

  let type_ = testable(Type.pp, (==));

  let target = testable(Target.pp, (==));

  exception LazyStreamLengthMismatch;

  let lazy_stream = (pp_value: Fmt.t('a)) =>
    testable(
      LazyStream.pp(pp_value),
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

  let namespace = testable(Reference.Namespace.pp, (==));

  let module_table = testable(ModuleTable.pp, ModuleTable.compare);
};

let int_trio =
  Alcotest.(check(Compare.trio(int, int, int), "int trio matches"));

let input = Alcotest.(check(Compare.input, "input matches"));

let point = Alcotest.(check(Compare.point, "point matches"));

let range = Alcotest.(check(Compare.range, "range matches"));

let graph = Alcotest.(check(Compare.graph(Fmt.string), "graph matches"));

let namespace_graph =
  Alcotest.(
    check(Compare.graph(Reference.Namespace.pp), "namespace graph matches")
  );

let untyped_node = pp_value =>
  Alcotest.(check(Compare.untyped_node(pp_value), "untyped node matches"));

let typed_node = (pp_value, pp_type) =>
  Alcotest.(
    check(Compare.typed_node(pp_value, pp_type), "typed node matches")
  );

let type_ = Alcotest.(check(Compare.type_, "type matches"));

let target = Alcotest.(check(Compare.target, "target matches"));
let opt_target = Alcotest.(check(option(Compare.target), "target matches"));

let lazy_stream = pp_value =>
  Alcotest.(check(Compare.lazy_stream(pp_value), "lazy stream matches"));

let namespace = Alcotest.(check(Compare.namespace, "namespace matches"));
let list_namespace =
  Alcotest.(check(list(Compare.namespace), "namespace list matches"));

let module_table =
  Alcotest.check(Compare.module_table, "module table matches");

let compile_errors =
  Alcotest.(
    check(testable(pp_dump_err_list, (==)), "compile error matches")
  );

let throws_compile_errors = (expected, f) =>
  switch (f()) {
  | exception (CompileError(errs)) => compile_errors(expected, errs)

  | _ =>
    Fmt.str("expected compile error(s) %a", pp_dump_err_list, expected)
    |> fail
  };
