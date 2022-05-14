open Kore;

let create = (~default=ConfigFile.defaults.out_dir, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="o",
      ~default=String(default),
      ~from_config=cfg => Some(String(cfg.out_dir)),
      out_dir_key,
      String(x => value := Some(x)),
      "the directory to write compiled files to",
    );
  /* no need to check if it exists since we can create it */
  let resolve = (cfg: option(Config.t), root_dir: string) =>
    value^
    |> Argument.resolve(cfg, x => x.out_dir, default)
    |> Filename.resolve(~cwd=root_dir)
    |> Filename.relative_to(root_dir);

  (argument, resolve);
};
