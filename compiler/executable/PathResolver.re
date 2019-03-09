open Kore;

let simple = (config, target) =>
  (
    switch (target) {
    | res when String.length(res) == 0 => raise(InvalidPathFormat(res))
    | res when Util.is_source_module(res) =>
      Util.to_path_segment(res)
      |> Printf.sprintf("%s.kn")
      |> Filename.concat(config.source_dir)
    | res when res.[0] == '@' =>
      let parts = String.split_on_char('/', res);

      if (List.length(parts) == 2) {
        let namespace = List.nth(parts, 0);
        let module_ = List.nth(parts, 1);

        Filename.concat(
          String.sub(namespace, 1, String.length(namespace) - 1),
          module_,
        )
        |> Filename.concat(config.module_dir);
      } else {
        raise(InvalidPathFormat(target));
      };
    | res when Filename.is_implicit(res) =>
      Filename.concat(config.module_dir, res)
    | res => res
    }
  )
  |> (
    s =>
      if (Sys.file_exists(s)) {
        s;
      } else if (Printf.sprintf("%sot", s) |> Sys.file_exists) {
        Printf.sprintf("%sot", s);
      } else {
        raise(ModuleDoesNotExist(target, s));
      }
  );
