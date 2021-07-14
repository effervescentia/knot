open Kore;

module Watcher = File.Watcher;

let __temp_dir = Util.get_temp_dir();
let __extensions = [".txt"];

let suite =
  "File.Watcher"
  >::: [
    "create()"
    >: (
      () => {
        let watcher = Watcher.create(__temp_dir, __extensions);

        watcher.dir |> Assert.string(__temp_dir);
        watcher.extensions |> Assert.string_list(__extensions);
      }
    ),
  ];
