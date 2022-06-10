open Kore;

let create = (~default=Config.defaults.root_dir, ()) => {
  let value = ref(None);
  let argument =
    Argument.create(
      ~alias="r",
      ~default=String(default),
      ~from_config=cfg => Some(String(cfg.root_dir)),
      root_dir_key,
      String(x => value := Some(x)),
      "the root directory used to resolve paths within the project",
    );
  let resolve = (cfg: option(Config.t), working_dir: string) => {
    let root_dir =
      value^
      |> Argument.resolve(cfg, x => x.root_dir, default)
      |> Filename.resolve(~cwd=working_dir);

    root_dir |> Util.assert_exists(root_dir_key);

    root_dir;
  };

  (argument, resolve);
};
