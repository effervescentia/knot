open Kore;

module ImportGraph = Resolve.ImportGraph;
module Nx = Fixtures.Namespace;
module Gx = Fixtures.Graph;

let _create_resolver = (~default=[], entries, id) =>
  entries |> List.assoc_opt(id) |?: default;

let _create_graph = imports => ImportGraph.{imports, get_imports: _ => []};

let suite =
  "Resolve.ImportGraph"
  >::: [
    "create()"
    >: (
      () => {
        let get_imports = _ => [];

        let import_graph = ImportGraph.create(get_imports);

        Assert.import_graph(
          {imports: Graph.empty(), get_imports},
          import_graph,
        );
      }
    ),
    "init()"
    >: (
      () => {
        let get_imports = _create_resolver([(Nx.foo, [Nx.bar])]);
        let import_graph = ImportGraph.create(get_imports);

        import_graph |> ImportGraph.init(Nx.foo);

        Assert.import_graph(
          {imports: Gx.two_node(), get_imports},
          import_graph,
        );
      }
    ),
    "add_module()"
    >: (
      () => {
        let import_graph =
          ImportGraph.{
            imports: Gx.two_node(),
            get_imports: _create_resolver([(Nx.fizz, [Nx.buzz, Nx.bar])]),
          };

        let added = import_graph |> ImportGraph.add_module(Nx.fizz);

        Assert.list_namespace([Nx.buzz, Nx.fizz], added);
        Assert.import_graph(
          {
            ...import_graph,
            imports:
              Graph.create(
                [Nx.buzz, Nx.fizz, Nx.bar, Nx.foo],
                [(Nx.fizz, Nx.bar), (Nx.fizz, Nx.buzz), (Nx.foo, Nx.bar)],
              ),
          },
          import_graph,
        );
      }
    ),
    "remove_module()"
    >: (
      () => {
        let import_graph = Gx.three_node() |> _create_graph;

        import_graph |> ImportGraph.remove_module(Nx.bar);

        Assert.import_graph(
          {
            ...import_graph,
            imports: Graph.create([Nx.fizz, Nx.foo], [(Nx.foo, Nx.bar)]),
          },
          import_graph,
        );
      }
    ),
    "prune_subtree()"
    >: (
      () => {
        let import_graph = Gx.three_node() |> _create_graph;

        let removed = import_graph |> ImportGraph.prune_subtree(Nx.bar);

        Assert.list_namespace([Nx.fizz, Nx.bar], removed);
        Assert.import_graph(
          {
            ...import_graph,
            imports: Graph.create([Nx.foo], [(Nx.foo, Nx.bar)]),
          },
          import_graph,
        );
      }
    ),
    "get_modules()"
    >: (
      () => {
        let import_graph = Gx.three_node() |> _create_graph;

        Assert.list_namespace(
          [Nx.fizz, Nx.bar, Nx.foo],
          ImportGraph.get_modules(import_graph),
        );
      }
    ),
    "has_module()"
    >: (
      () => {
        let import_graph = Gx.three_node() |> _create_graph;

        Assert.true_(import_graph |> ImportGraph.has_module(Nx.foo));
        Assert.false_(import_graph |> ImportGraph.has_module(Nx.buzz));
      }
    ),
    "find_missing()"
    >: (
      () => {
        let import_graph = Gx.three_node() |> _create_graph;

        import_graph |> ImportGraph.prune_subtree(Nx.bar) |> ignore;

        Assert.list_namespace(
          [Nx.bar],
          import_graph |> ImportGraph.find_missing,
        );
      }
    ),
    "clear()"
    >: (
      () => {
        let import_graph = Gx.three_node() |> _create_graph;

        ImportGraph.clear(import_graph);

        Assert.import_graph(
          {...import_graph, imports: Graph.empty()},
          import_graph,
        );
      }
    ),
    "pp()"
    >: (
      () => {
        let import_graph = Gx.two_node() |> _create_graph;

        Assert.string(
          "@/foo \n\
|     \n\
bar   \n\      ",
          import_graph |> ~@ImportGraph.pp,
        );
      }
    ),
  ];
