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
let __converging_graph = Graph.invert(__branching_graph);
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

let suite =
  "Library.Graph"
  >::: [
    "empty()" >: (() => Assert.graph({nodes: [], edges: []}, Graph.empty())),
    "create()"
    >: (
      () =>
        Assert.graph(
          __acyclic_graph,
          Graph.create(
            ["foo", "bar", "fizz", "buzz"],
            [("foo", "fizz"), ("buzz", "fizz"), ("bar", "foo")],
          ),
        )
    ),
    "get_nodes()"
    >: (
      () =>
        Assert.string_list(
          ["foo", "bar", "fizz", "buzz"],
          Graph.get_nodes(__acyclic_graph),
        )
    ),
    "get_edges()"
    >: (
      () =>
        Assert.string_pair_list(
          [("foo", "fizz"), ("buzz", "fizz"), ("bar", "foo")],
          Graph.get_edges(__acyclic_graph),
        )
    ),
    "has_node() - does have node"
    >: (() => Assert.true_(Graph.has_node("fizz", __acyclic_graph))),
    "has_node() - does not have node"
    >: (() => Assert.false_(Graph.has_node("foobar", __acyclic_graph))),
    "get_edges_of()"
    >: (
      () =>
        Assert.string_pair_list(
          [("buzz", "fizz"), ("foo", "fizz")],
          Graph.get_edges_of("fizz", __acyclic_graph),
        )
    ),
    "get_neighbors()"
    >: (
      () =>
        Assert.string_list(
          ["buzz", "foo"],
          Graph.get_neighbors("fizz", __acyclic_graph),
        )
    ),
    "get_parents()"
    >: (
      () =>
        Assert.string_list(
          ["buzz", "foo"],
          Graph.get_parents("fizz", __acyclic_graph),
        )
    ),
    "get_ancestors() - acyclic branching"
    >: (
      () =>
        Assert.string_list(
          ["b", "a"],
          __branching_graph |> Graph.get_ancestors("d"),
        )
    ),
    "get_ancestors() - acyclic converging"
    >: (
      () =>
        Assert.string_list(
          ["c", "e", "d", "g", "f"],
          __converging_graph |> Graph.get_ancestors("b"),
        )
    ),
    "get_ancestors() - cyclic"
    >: (
      () =>
        Assert.string_list(
          ["bar", "foe", "foo", "fizz"],
          __cyclic_graph |> Graph.get_ancestors("fie"),
        )
    ),
    "get_children()"
    >: (
      () =>
        Assert.string_list(
          ["fizz"],
          Graph.get_children("foo", __acyclic_graph),
        )
    ),
    "get_descendants() - acyclic branching"
    >: (
      () =>
        Assert.string_list(
          ["c", "b", "g", "f", "e", "d"],
          __branching_graph |> Graph.get_descendants("a"),
        )
    ),
    "get_descendants() - acyclic converging"
    >: (
      () =>
        Assert.string_list(
          ["b", "a"],
          __converging_graph |> Graph.get_descendants("d"),
        )
    ),
    "get_descendants() - cyclic"
    >: (
      () =>
        Assert.string_list(
          ["foe", "bar", "fizz", "foo"],
          __cyclic_graph |> Graph.get_descendants("fie"),
        )
    ),
    "add_node()"
    >: (
      () => {
        let graph = Graph.create(["fizz"], []);

        Graph.add_node("buzz", graph);

        Assert.graph({nodes: ["buzz", "fizz"], edges: []}, graph);
      }
    ),
    "add_edge() - add new edge"
    >: (
      () => {
        let graph =
          Graph.create(["foo", "bar", "fizz", "buzz"], [("foo", "buzz")]);

        Graph.add_edge("buzz", "bar", graph);

        Assert.graph(
          {
            nodes: ["foo", "bar", "fizz", "buzz"],
            edges: [("buzz", "bar"), ("foo", "buzz")],
          },
          graph,
        );
      }
    ),
    "add_edge() - add existing edge"
    >: (
      () => {
        let graph = _make_graph();

        Graph.add_edge("foo", "fizz", graph);

        Assert.graph(__acyclic_graph, graph);
      }
    ),
    "add_edge() - add invalid edge"
    >: (
      () =>
        Assert.throws(Graph.InvalidEdge, "should raise InvalidEdge", () =>
          Graph.add_edge("hi", "low", _make_graph())
        )
    ),
    "union()"
    >: (
      () => {
        let graph =
          Graph.union(
            {
              nodes: ["foo", "bar", "buzz"],
              edges: [("buzz", "foo"), ("bar", "foo")],
            },
            {
              nodes: ["foo", "fizz", "buzz", "foobar"],
              edges: [
                ("foo", "fizz"),
                ("buzz", "fizz"),
                ("buzz", "foo"),
                ("bar", "foobar"),
              ],
            },
          );

        Assert.graph(
          {
            nodes: ["foobar", "fizz", "buzz", "bar", "foo"],
            edges: [
              ("bar", "foobar"),
              ("buzz", "fizz"),
              ("foo", "fizz"),
              ("bar", "foo"),
              ("buzz", "foo"),
            ],
          },
          graph,
        );
      }
    ),
    "remove_node()"
    >: (
      () => {
        let graph = _make_graph();

        Graph.remove_node("buzz", graph);

        Assert.graph(
          {
            nodes: ["foo", "bar", "fizz"],
            edges: [("foo", "fizz"), ("buzz", "fizz"), ("bar", "foo")],
          },
          graph,
        );
      }
    ),
    "remove_edges_to()"
    >: (
      () => {
        let graph = _make_graph();

        Graph.remove_edges_to("fizz", graph);

        Assert.graph(
          {nodes: ["foo", "bar", "fizz", "buzz"], edges: [("bar", "foo")]},
          graph,
        );
      }
    ),
    "remove_edges_from()"
    >: (
      () => {
        let graph = _make_graph();

        Graph.remove_edges_from("buzz", graph);

        Assert.graph(
          {
            nodes: ["foo", "bar", "fizz", "buzz"],
            edges: [("foo", "fizz"), ("bar", "foo")],
          },
          graph,
        );
      }
    ),
    "find_cycles() - has cycles"
    >: (
      () =>
        __cyclic_graph
        |> Graph.find_cycles("foo")
        |> _assert_cycles([["bar", "fizz", "foo"]])
    ),
    "find_cycles() - no cycles"
    >: (
      () => _make_graph() |> Graph.find_cycles("foo") |> _assert_cycles([])
    ),
    "find_unique_cycles()"
    >: (
      () =>
        __cyclic_graph
        |> Graph.find_unique_cycles(["foo", "bar"])
        |> _assert_cycles([["fie", "foe", "bar"], ["bar", "fizz", "foo"]])
    ),
    "find_all_unique_cycles()"
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
    "is_acyclic() - has no cycles"
    >: (() => Assert.true_(Graph.is_acyclic(__acyclic_graph))),
    "is_acyclic() - has cycles"
    >: (() => Assert.false_(Graph.is_acyclic(__cyclic_graph))),
    "pp()"
    >: (
      () =>
        Assert.string(
          "a                   \n\
|------,            \n\
|      |            \n\
c      b            \n\
|-,-,  |------,-,   \n\
| | |  |      | |   \n\
g f e  c      e d   \n       |            \n       […]        "
          |> String.replace(' ', 'x'),
          __branching_graph
          |> ~@Graph.pp(Fmt.string)
          |> String.replace(' ', 'x'),
        )
    ),
  ];
