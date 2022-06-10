open Kore;

let create = (~default=?, ()) => {
  let value = ref(default);
  let argument =
    Argument.create(
      ~alias="c",
      ~default=?default |?> (x => Argument.Value.String(x)),
      config_key,
      String(path => value := Some(path)),
      "the location of a knot config file",
    );
  let resolve = (working_dir: string) => {
    let config = value^ |?> Filename.resolve(~cwd=working_dir);

    config |> Option.iter(Util.assert_exists(config_key));

    config;
  };

  (argument, resolve);
};
