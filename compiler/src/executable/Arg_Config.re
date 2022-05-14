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
  let resolve = () => value^;

  (argument, resolve);
};
