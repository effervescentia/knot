open Kore;

module ImportGraph = Resolve.ImportGraph;
module N = Fixtures.Namespace;

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
        let get_imports = _create_resolver([(N.foo, [N.bar])]);
        let import_graph = ImportGraph.create(get_imports);

        import_graph |> ImportGraph.init(N.foo);

        Assert.import_graph(
          {imports: Fixtures.Graph.two_node(), get_imports},
          import_graph,
        );
      }
    ),
    "add_module()"
    >: (
      () => {
        let import_graph =
          ImportGraph.{
            imports: Fixtures.Graph.two_node(),
            get_imports: _create_resolver([(N.fizz, [N.buzz, N.bar])]),
          };

        let added = import_graph |> ImportGraph.add_module(N.fizz);

        Assert.list_namespace([N.buzz, N.fizz], added);
        Assert.import_graph(
          {
            ...import_graph,
            imports:
              Graph.create(
                [N.buzz, N.fizz, N.bar, N.foo],
                [(N.fizz, N.bar), (N.fizz, N.buzz), (N.foo, N.bar)],
              ),
          },
          import_graph,
        );
      }
    ),
    "prune_subtree()"
    >: (
      () => {
        let import_graph = Fixtures.Graph.three_node() |> _create_graph;

        let removed = import_graph |> ImportGraph.prune_subtree(N.bar);

        Assert.list_namespace([N.fizz, N.bar], removed);
        Assert.import_graph(
          {
            ...import_graph,
            imports: Graph.create([N.foo], [(N.foo, N.bar)]),
          },
          import_graph,
        );
      }
    ),
    "find_missing()"
    >: (
      () => {
        let import_graph = Fixtures.Graph.three_node() |> _create_graph;

        import_graph |> ImportGraph.prune_subtree(N.bar) |> ignore;

        Assert.list_namespace(
          [N.bar],
          import_graph |> ImportGraph.find_missing,
        );
      }
    ),
    "refresh_subtree()"
    >: (
      () => {
        let import_graph =
          ImportGraph.{
            imports: Fixtures.Graph.three_node(),
            get_imports:
              _create_resolver(~default=[N.buzz], [(N.buzz, [])]),
          };

        let (removed, added) =
          import_graph |> ImportGraph.refresh_subtree(N.bar);

        Assert.list_namespace([N.fizz], removed);
        Assert.list_namespace([N.buzz, N.bar], added);
        Assert.import_graph(
          {
            ...import_graph,
            imports:
              Graph.create(
                [N.buzz, N.bar, N.foo],
                [(N.bar, N.buzz), (N.foo, N.bar)],
              ),
          },
          import_graph,
        );
      }
    ),
    "pp()"
    >: (
      () => {
        let import_graph = Fixtures.Graph.two_node() |> _create_graph;

        Assert.string(
          "@/foo \n\
|     \n\
bar   \n\      ",
          import_graph |> ~@ImportGraph.pp,
        );
      }
    ),
  ];
