open Kore;

let _make_graph = () =>
  Graph.create(
    ["foo", "bar", "fizz", "buzz"],
    [("foo", "fizz"), ("buzz", "fizz"), ("bar", "foo")],
  );
let __acyclic_graph = _make_graph();
let __branching_graph =
  Graph.create(
    ["a", "b", "c", "d", "e", "f", "g"],
    [
      ("a", "b"),
      ("a", "c"),
      ("b", "d"),
      ("b", "e"),
      ("c", "e"),
      ("c", "f"),
      ("c", "g"),
      ("b", "c"),
    ],
  );
let __cyclic_graph =
  Graph.create(
    ["foo", "bar", "fizz", "buzz", "fee", "fie", "foe"],
    [
      ("foo", "bar"),
      ("bar", "fizz"),
      ("fizz", "foo"),
      ("buzz", "fee"),
      ("fee", "buzz"),
      ("fie", "foe"),
      ("foe", "bar"),
      ("bar", "fie"),
    ],
  );

let _assert_cycles = Alcotest.(check(list(list(string)), "cycles match"));

module GraphAssert =
  Assert.Make({
    type t = Graph.t(string);

    let test =
      Alcotest.(
        check(
          testable(
            pp =>
              Graph.to_string(Functional.identity)
              % Format.pp_print_string(pp),
            (==),
          ),
          "graph matches",
        )
      );
  });

let suite =
  "Library - Graph"
  >::: [
    "empty"
    >: (() => GraphAssert.test({nodes: [], edges: []}, Graph.empty())),
    "create"
    >: (
      () =>
        GraphAssert.test(
          __acyclic_graph,
          Graph.create(
            ["foo", "bar", "fizz", "buzz"],
            [("foo", "fizz"), ("buzz", "fizz"), ("bar", "foo")],
          ),
        )
    ),
    "get nodes"
    >: (
      () =>
        Assert.string_list(
          ["foo", "bar", "fizz", "buzz"],
          Graph.get_nodes(__acyclic_graph),
        )
    ),
    "get edges"
    >: (
      () =>
        Assert.string_pair_list(
          [("foo", "fizz"), ("buzz", "fizz"), ("bar", "foo")],
          Graph.get_edges(__acyclic_graph),
        )
    ),
    "has nodes"
    >: (
      () => {
        Graph.has_node("fizz", __acyclic_graph) |> Assert.true_;
        Graph.has_node("foobar", __acyclic_graph) |> Assert.false_;
      }
    ),
    "get edges of"
    >: (
      () =>
        Assert.string_pair_list(
          [("buzz", "fizz"), ("foo", "fizz")],
          Graph.get_edges_of("fizz", __acyclic_graph),
        )
    ),
    "get neighbors"
    >: (
      () =>
        Assert.string_list(
          ["buzz", "foo"],
          Graph.get_neighbors("fizz", __acyclic_graph),
        )
    ),
    "get parents"
    >: (
      () =>
        Assert.string_list(
          ["buzz", "foo"],
          Graph.get_parents("fizz", __acyclic_graph),
        )
    ),
    "get children"
    >: (
      () =>
        Assert.string_list(
          ["fizz"],
          Graph.get_children("foo", __acyclic_graph),
        )
    ),
    "add node"
    >: (
      () => {
        let graph = Graph.create(["fizz"], []);

        Graph.add_node("buzz", graph);

        GraphAssert.test({nodes: ["buzz", "fizz"], edges: []}, graph);
      }
    ),
    "add edge"
    >: (
      () => {
        let graph =
          Graph.create(["foo", "bar", "fizz", "buzz"], [("foo", "buzz")]);

        Graph.add_edge("buzz", "bar", graph);

        GraphAssert.test(
          {
            nodes: ["foo", "bar", "fizz", "buzz"],
            edges: [("buzz", "bar"), ("foo", "buzz")],
          },
          graph,
        );
      }
    ),
    "add existing edge"
    >: (
      () => {
        let graph = _make_graph();

        Graph.add_edge("foo", "fizz", graph);

        GraphAssert.test(__acyclic_graph, graph);
      }
    ),
    "add invalid edge"
    >: (
      () =>
        switch (Graph.add_edge("hi", "low", _make_graph())) {
        | exception Graph.InvalidEdge => ()
        | _ => Alcotest.fail("expected exception")
        }
    ),
    "remove node"
    >: (
      () => {
        let graph = _make_graph();

        Graph.remove_node("buzz", graph);

        GraphAssert.test(
          {
            nodes: ["foo", "bar", "fizz"],
            edges: [("foo", "fizz"), ("buzz", "fizz"), ("bar", "foo")],
          },
          graph,
        );
      }
    ),
    "find cycles"
    >: (
      () =>
        __cyclic_graph
        |> Graph.find_cycles("foo")
        |> _assert_cycles([["bar", "fizz", "foo"]])
    ),
    "find no cycles"
    >: (
      () => _make_graph() |> Graph.find_cycles("foo") |> _assert_cycles([])
    ),
    "find unique cycles"
    >: (
      () =>
        __cyclic_graph
        |> Graph.find_unique_cycles(["foo", "bar"])
        |> _assert_cycles([["fie", "foe", "bar"], ["bar", "fizz", "foo"]])
    ),
    "find all unique cycles"
    >: (
      () =>
        __cyclic_graph
        |> Graph.find_all_unique_cycles
        |> _assert_cycles([
             ["fee", "buzz"],
             ["fie", "foe", "bar"],
             ["bar", "fizz", "foo"],
           ])
    ),
    "is acyclic"
    >: (
      () => {
        __acyclic_graph |> Graph.is_acyclic |> Assert.true_;
        __cyclic_graph |> Graph.is_acyclic |> Assert.false_;
      }
    ),
  ];
