open Kore;

let create = (~default=?, ()) => {
  let value = ref(default);
  let argument =
    Argument.create(
      ~alias="c",
      ~default=?default |> Option.map(x => Argument.Value.String(x)),
      config_key,
      String(path => value := Some(path)),
      "the location of a knot config file",
    );
  let resolve = (working_dir: string) =>
    value^
    |> Option.map(path => {
         let absolute_path = path |> Filename.resolve(~cwd=working_dir);

         absolute_path |> Util.assert_exists(config_key);

         absolute_path;
       });

  (argument, resolve);
};
