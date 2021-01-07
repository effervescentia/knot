open Kore;

type mode =
  | Static
  | Server;

let () = {
  let mode = Static;
  let name = "";
  let config = Config.from_args();

  let root_dir = config.root_dir;
  let entry = Internal(config.entry);

  Log.init(config.debug);

  try(
    switch (mode) {
    | Static => Static.run({name, root_dir, entry})
    | Server =>
      Server.run({name, root_dir, port: 8080, watch: Some({entry: entry})})
    }
  ) {
  | CyclicImports(cycles) =>
    Log.error("found the following import cycles:");
    cycles
    |> List.map(Print.many(~separator=", ", Functional.identity))
    |> Print.many(~separator="\n", Print.fmt("[%s]"))
    |> Log.error("[%s]");
  };
};
